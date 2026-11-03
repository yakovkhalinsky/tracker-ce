#include "mtGenerators.h"

namespace mtGen
{

namespace
{

inline int clampInt(int v, int lo, int hi)
{
    return (v < lo) ? lo : (hi < v) ? hi : v;
}

inline void orderRange(int& lo, int& hi)
{
    if (hi < lo)
    {
        int t = lo;
        lo = hi;
        hi = t;
    }
    lo = clampInt(lo, kNoteMin, kNoteMax);
    hi = clampInt(hi, kNoteMin, kNoteMax);
}

inline int semitoneFromRoot(int note, uint8_t root)
{
    int n = note % 12;
    int r = (int)root % 12;
    int semis = n - r;
    if (semis < 0)
        semis += 12;
    return semis;
}

int nthInScale(int index, int lo, int hi, uint8_t root, uint16_t scaleMask)
{
    for (int n = lo; n <= hi; n++)
    {
        if (inScale(n, root, scaleMask))
        {
            if (index == 0)
                return n;
            index--;
        }
    }
    return lo;
}

int randomInScale(Rng& rng, int lo, int hi, uint8_t root, uint16_t scaleMask)
{
    int count = countInScale(lo, hi, root, scaleMask);
    if (count <= 0)
        return lo;
    return nthInScale(rng.range(0, count - 1), lo, hi, root, scaleMask);
}

} // namespace

bool inScale(int note, uint8_t root, uint16_t scaleMask)
{
    if (note < kNoteMin || note > kNoteMax)
        return false;
    return ((scaleMask >> semitoneFromRoot(note, root)) & 1u) != 0;
}

int nearestInScale(int note, int lo, int hi, uint8_t root, uint16_t scaleMask)
{
    orderRange(lo, hi);
    if (lo > hi)
        return kNoteMin;

    int n = clampInt(note, lo, hi);
    for (int d = 0; d <= kNoteMax; d++)
    {
        int a = n - d;
        if (a >= lo && a <= hi && inScale(a, root, scaleMask))
            return a;
        int b = n + d;
        if (b != a && b >= lo && b <= hi && inScale(b, root, scaleMask))
            return b;
    }
    return lo;
}

int countInScale(int lo, int hi, uint8_t root, uint16_t scaleMask)
{
    orderRange(lo, hi);
    int count = 0;
    for (int n = lo; n <= hi; n++)
    {
        if (inScale(n, root, scaleMask))
            count++;
    }
    return count;
}

//----------------------------------------------------------------------------
// Walk
//----------------------------------------------------------------------------
void walkInit(WalkState& state, uint32_t seed)
{
    state.rng.reseed(seed);
    state.note = 0;
    state.started = false;
}

int walkNext(WalkState& state,
             int lo,
             int hi,
             int maxJump,
             uint8_t root,
             uint16_t scaleMask)
{
    orderRange(lo, hi);
    maxJump = clampInt(maxJump, 0, kNoteMax);

    if (countInScale(lo, hi, root, scaleMask) <= 0)
    {
        state.started = true;
        state.note = lo;
        return lo;
    }

    if (!state.started)
    {
        state.note = randomInScale(state.rng, lo, hi, root, scaleMask);
        state.started = true;
        return state.note;
    }

    const int jump = state.rng.range(-maxJump, maxJump);
    const int target = clampInt(state.note + jump, lo, hi);

    // Pick the in-scale note closest to `target` that is still reachable from
    // the previous note with one jump.  This keeps maxJump true also after
    // scale quantisation.
    int best = -1;
    int bestDistance = kNoteMax + 1;
    const int reachLo = state.note - maxJump;
    const int reachHi = state.note + maxJump;
    for (int n = reachLo; n <= reachHi; n++)
    {
        if (n < lo || n > hi || !inScale(n, root, scaleMask))
            continue;
        int d = n - target;
        if (d < 0)
            d = -d;
        if (d < bestDistance)
        {
            bestDistance = d;
            best = n;
        }
    }

    if (best < 0)
        best = nearestInScale(state.note, lo, hi, root, scaleMask);

    state.note = best;
    return best;
}

//----------------------------------------------------------------------------
// Turing
//----------------------------------------------------------------------------
void turingReset(TuringState& state)
{
    state.rng.reseed(1);
    state.reg = 0;
    state.length = 8;
    state.seeded = false;
}

void turingSeed(TuringState& state, int length, uint32_t seed)
{
    state.length = (uint8_t)clampInt(length, 1, kMaxRegisterLength);
    state.rng.reseed(seed);

    const uint32_t mask = (1u << state.length) - 1u;
    state.reg = (uint16_t)(state.rng.next() & mask);
    if (state.reg == 0)
        state.reg = 1; // a zero register would output one note forever
    state.seeded = true;
}

int turingNoteForRegister(uint16_t reg, int lo, int hi, uint8_t root, uint16_t scaleMask)
{
    orderRange(lo, hi);
    const int count = countInScale(lo, hi, root, scaleMask);
    if (count <= 0)
        return lo;
    return nthInScale((int)(reg % (uint16_t)count), lo, hi, root, scaleMask);
}

int turingNext(TuringState& state,
               int lo,
               int hi,
               int lockPercent,
               uint8_t root,
               uint16_t scaleMask)
{
    orderRange(lo, hi);

    if (!state.seeded)
        turingSeed(state, state.length, 1);

    state.length = (uint8_t)clampInt(state.length, 1, kMaxRegisterLength);
    const uint32_t mask = (1u << state.length) - 1u;

    const int note = turingNoteForRegister(state.reg, lo, hi, root, scaleMask);

    // clock: rotate left one bit
    uint32_t r = state.reg & mask;
    r = ((r << 1) | (r >> (state.length - 1))) & mask;

    // mutate: every bit flips with probability (100 - lock)%
    const int flipPercent = 100 - clampInt(lockPercent, 0, 100);
    if (flipPercent > 0)
    {
        for (int b = 0; b < state.length; b++)
        {
            if ((int)(state.rng.next() % 100u) < flipPercent)
                r ^= (1u << b);
        }
    }

    state.reg = (uint16_t)(r & mask);
    return note;
}

//----------------------------------------------------------------------------
// Markov
//----------------------------------------------------------------------------
int markovGenerate(const int8_t* source,
                   int sourceLen,
                   int order,
                   int outCount,
                   int lo,
                   int hi,
                   uint8_t root,
                   uint16_t scaleMask,
                   uint32_t seed,
                   int8_t* out)
{
    if (source == nullptr || sourceLen <= 0 || outCount <= 0 || out == nullptr)
        return 0;

    orderRange(lo, hi);
    order = clampInt(order, 1, kMaxMarkovOrder);
    outCount = clampInt(outCount, 0, kMaxSteps);

    // Drop rests/note-offs so transitions connect across them.
    uint8_t clean[kMaxSteps];
    int cleanLen = 0;
    for (int i = 0; i < sourceLen && cleanLen < kMaxSteps; i++)
    {
        if (source[i] >= 0)
            clean[cleanLen++] = (uint8_t)source[i];
    }
    if (cleanLen <= 0)
        return 0;

    auto allowed = [lo, hi, root, scaleMask](int note) {
        return note >= lo && note <= hi && inScale(note, root, scaleMask);
    };

    int allowedCount = 0;
    for (int i = 0; i < cleanLen; i++)
    {
        if (allowed(clean[i]))
            allowedCount++;
    }
    if (allowedCount <= 0)
        return 0;

    Rng rng(seed);

    auto pickAllowed = [&]() {
        int k = rng.range(0, allowedCount - 1);
        for (int i = 0; i < cleanLen; i++)
        {
            if (!allowed(clean[i]))
                continue;
            if (k == 0)
                return (int)clean[i];
            k--;
        }
        return (int)clean[0];
    };

    int ctx[kMaxMarkovOrder] = { 0, 0, 0, 0 };
    int ctxCount = 0;

    int produced = 0;

    // Start from a real source context that has an allowed successor, so the
    // first generated note is itself a learned transition.
    int startCount = 0;
    for (int q = order - 1; q < cleanLen - 1; q++)
    {
        bool match = true;
        for (int k = 0; k < order; k++)
        {
            if (!allowed(clean[q - order + 1 + k]))
            {
                match = false;
                break;
            }
        }
        if (match && allowed(clean[q + 1]))
            startCount++;
    }

    if (startCount > 0)
    {
        int pick = rng.range(0, startCount - 1);
        int startQ = order - 1;
        for (int q = order - 1; q < cleanLen - 1; q++)
        {
            bool match = true;
            for (int k = 0; k < order; k++)
            {
                if (!allowed(clean[q - order + 1 + k]))
                {
                    match = false;
                    break;
                }
            }
            if (!match || !allowed(clean[q + 1]))
                continue;
            if (pick == 0)
            {
                startQ = q;
                break;
            }
            pick--;
        }

        for (int k = 0; k < order; k++)
            ctx[k] = clean[startQ - order + 1 + k];
        ctxCount = order;

        out[produced++] = (int8_t)clean[startQ + 1];
        for (int k = 0; k < order - 1; k++)
            ctx[k] = ctx[k + 1];
        ctx[order - 1] = out[produced - 1];
    }
    else
    {
        out[produced++] = (int8_t)pickAllowed();
        ctx[ctxCount++] = out[produced - 1];
    }

    while (produced < outCount)
    {
        int nextNote = -1;

        if (ctxCount < order)
        {
            nextNote = pickAllowed();
        }
        else
        {
            // Count the successors of the current context that pass the
            // range/scale filter.
            int matches = 0;
            for (int q = order - 1; q < cleanLen - 1; q++)
            {
                bool match = true;
                for (int k = 0; k < order; k++)
                {
                    if ((int)clean[q - order + 1 + k] != ctx[k])
                    {
                        match = false;
                        break;
                    }
                }
                if (match && allowed(clean[q + 1]))
                    matches++;
            }

            if (matches > 0)
            {
                int pick = rng.range(0, matches - 1);
                for (int q = order - 1; q < cleanLen - 1; q++)
                {
                    bool match = true;
                    for (int k = 0; k < order; k++)
                    {
                        if ((int)clean[q - order + 1 + k] != ctx[k])
                        {
                            match = false;
                            break;
                        }
                    }
                    if (!match || !allowed(clean[q + 1]))
                        continue;
                    if (pick == 0)
                    {
                        nextNote = clean[q + 1];
                        break;
                    }
                    pick--;
                }
            }
            else
            {
                // No learned transition fits the range/scale filter: restart
                // from a random allowed source note.
                ctxCount = 0;
                nextNote = pickAllowed();
            }
        }

        if (nextNote < 0)
            break;

        out[produced++] = (int8_t)nextNote;

        if (ctxCount < order)
        {
            ctx[ctxCount++] = nextNote;
        }
        else
        {
            for (int k = 0; k < order - 1; k++)
                ctx[k] = ctx[k + 1];
            ctx[order - 1] = nextNote;
        }
    }

    return produced;
}

//----------------------------------------------------------------------------
// Prob / Ratchet value mapping
//----------------------------------------------------------------------------
int chanceFromPercent(int percent)
{
    return clampInt(percent, 0, kChanceProbMax);
}

int rollPeriodFromCount(int count)
{
    return clampInt(count, kRollPeriodMin, kRollPeriodMax);
}

} // namespace mtGen
