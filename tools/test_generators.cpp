// Host unit tests for the generative fill algorithms.
//
// Build and run from the repository root:
//     make gen-test
//
// The tests only need g++ and the two files core/source/mtGenerators.{h,cpp}.

#include "mtGenerators.h"
#include "scales.h"

#include <cmath>
#include <cstdio>
#include <cstring>
#include <initializer_list>

static int failures = 0;
static int checks = 0;

#define CHECK(cond)                                                                                \
    do                                                                                             \
    {                                                                                              \
        checks++;                                                                                  \
        if (!(cond))                                                                               \
        {                                                                                          \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);                            \
            failures++;                                                                            \
        }                                                                                          \
    } while (0)

static bool refInScale(int note, uint8_t root, uint16_t scaleMask)
{
    int noteName = note % 12;
    int rootName = root % 12;
    if (noteName == rootName)
        return true;
    if (noteName > rootName)
        return (scaleMask & (1 << (noteName - rootName))) != 0;
    return (scaleMask & (1 << ((12 - rootName) + noteName))) != 0;
}

static const uint16_t kChromatic = 0x0FFF;

static void testScales()
{
    for (int scale = 0; scale <= MAX_SCALE; scale++)
    {
        for (int root = 0; root < 12; root++)
        {
            for (int note = 0; note <= 127; note++)
            {
                CHECK(mtGen::inScale(note, (uint8_t)root, scaleDef[scale]) ==
                      refInScale(note, (uint8_t)root, scaleDef[scale]));
            }
        }
    }

    // nearestInScale always returns an in-range, in-scale note.
    const uint16_t major = scaleDef[2];
    for (int lo = 0; lo < 120; lo += 7)
    {
        for (int hi = lo; hi < 128; hi += 11)
        {
            for (int note = -5; note < 135; note += 3)
            {
                int q = mtGen::nearestInScale(note, lo, hi, 0, major);
                CHECK(q >= lo && q <= hi);
                if (mtGen::countInScale(lo, hi, 0, major) > 0)
                {
                    CHECK(mtGen::inScale(q, 0, major));
                    // no closer in-scale note exists
                    for (int n = lo; n <= hi; n++)
                    {
                        if (mtGen::inScale(n, 0, major))
                            CHECK(std::abs(n - note) >= std::abs(q - note));
                    }
                }
            }
        }
    }
}

static void testWalk()
{
    const uint16_t scales[3] = { kChromatic, scaleDef[2] /* major */, scaleDef[1] /* minor */ };
    const int ranges[4][2] = { { 0, 127 }, { 48, 72 }, { 60, 60 }, { 10, 20 } };
    const int jumps[4] = { 0, 1, 2, 12 };
    const uint32_t seeds[5] = { 1u, 2u, 3u, 12345u, 0xDEADBEEFu };

    for (uint16_t mask : scales)
    {
        for (auto& range : ranges)
        {
            for (int maxJump : jumps)
            {
                for (uint32_t seed : seeds)
                {
                    mtGen::WalkState a;
                    mtGen::WalkState b;
                    mtGen::walkInit(a, seed);
                    mtGen::walkInit(b, seed);

                    int previous = -1;
                    for (int i = 0; i < 500; i++)
                    {
                        int lo = range[0];
                        int hi = range[1];
                        int na = mtGen::walkNext(a, lo, hi, maxJump, 0, mask);
                        int nb = mtGen::walkNext(b, lo, hi, maxJump, 0, mask);

                        CHECK(na == nb); // deterministic for the same seed
                        CHECK(na >= lo && na <= hi);
                        CHECK(mtGen::inScale(na, 0, mask));
                        if (previous >= 0)
                            CHECK(std::abs(na - previous) <= maxJump);
                        previous = na;
                    }
                }
            }
        }
    }

    // Different seeds produce different walks.
    mtGen::WalkState a;
    mtGen::WalkState b;
    mtGen::walkInit(a, 1);
    mtGen::walkInit(b, 2);
    bool differs = false;
    for (int i = 0; i < 64; i++)
    {
        int na = mtGen::walkNext(a, 0, 127, 4, 0, kChromatic);
        int nb = mtGen::walkNext(b, 0, 127, 4, 0, kChromatic);
        if (na != nb)
            differs = true;
    }
    CHECK(differs);

    // maxJump == 0 is a constant walk.
    mtGen::WalkState c;
    mtGen::walkInit(c, 7);
    int first = mtGen::walkNext(c, 40, 80, 0, 0, kChromatic);
    for (int i = 0; i < 32; i++)
        CHECK(mtGen::walkNext(c, 40, 80, 0, 0, kChromatic) == first);
}

static void testTuring()
{
    const int lengths[5] = { 1, 2, 4, 8, 16 };
    const uint32_t seeds[4] = { 1u, 7u, 1234u, 0xCAFEBABEu };
    const uint16_t masks[2] = { kChromatic, scaleDef[2] };

    for (int length : lengths)
    {
        for (uint32_t seed : seeds)
        {
            for (uint16_t mask : masks)
            {
                // Deterministic for the same seed.
                mtGen::TuringState a;
                mtGen::TuringState b;
                mtGen::turingSeed(a, length, seed);
                mtGen::turingSeed(b, length, seed);

                int seqA[4 * 16];
                int seqB[4 * 16];
                for (int i = 0; i < 4 * length; i++)
                {
                    seqA[i] = mtGen::turingNext(a, 36, 84, 100, 0, mask);
                    seqB[i] = mtGen::turingNext(b, 36, 84, 100, 0, mask);
                    CHECK(seqA[i] == seqB[i]);
                    CHECK(seqA[i] >= 36 && seqA[i] <= 84);
                    CHECK(mtGen::inScale(seqA[i], 0, mask));
                }

                // Lock = 100: the register only rotates, so the note sequence
                // repeats every `length` steps.
                for (int i = 0; i + length < 4 * length; i++)
                    CHECK(seqA[i] == seqA[i + length]);

                // Lock = 100: after `length` clocks the register is unchanged.
                mtGen::TuringState c;
                mtGen::turingSeed(c, length, seed);
                uint16_t regStart = c.reg;
                for (int i = 0; i < length; i++)
                    mtGen::turingNext(c, 36, 84, 100, 0, mask);
                CHECK(c.reg == regStart);
            }
        }
    }

    // Lock < 100 mutates the register.
    for (uint32_t seed : seeds)
    {
        mtGen::TuringState s;
        mtGen::turingSeed(s, 8, seed);
        uint16_t regStart = s.reg;
        int seq[64];
        for (int i = 0; i < 64; i++)
            seq[i] = mtGen::turingNext(s, 36, 84, 50, 0, kChromatic);
        CHECK(s.reg != regStart);
        CHECK(s.reg != 0);

        // and it is not the pure-rotation loop
        bool periodic = true;
        for (int i = 0; i + 8 < 64; i++)
            if (seq[i] != seq[i + 8])
                periodic = false;
        CHECK(!periodic);
    }

    // Lock = 0 mutates much faster.
    mtGen::TuringState zero;
    mtGen::turingSeed(zero, 16, 99);
    uint16_t zeroStart = zero.reg;
    for (int i = 0; i < 4; i++)
        mtGen::turingNext(zero, 0, 127, 0, 0, kChromatic);
    CHECK(zero.reg != zeroStart);

    // Register mapping respects range and scale.
    for (uint16_t reg = 0; reg < 1000; reg += 37)
    {
        int note = mtGen::turingNoteForRegister(reg, 48, 72, 0, scaleDef[1]);
        CHECK(note >= 48 && note <= 72);
        CHECK(mtGen::inScale(note, 0, scaleDef[1]));
    }
}

static bool containsSubsequence(const int8_t* source, int sourceLen, const int* pattern, int patternLen)
{
    for (int i = 0; i + patternLen <= sourceLen; i++)
    {
        bool match = true;
        for (int k = 0; k < patternLen; k++)
        {
            if (source[i + k] != pattern[k])
            {
                match = false;
                break;
            }
        }
        if (match)
            return true;
    }
    return false;
}

static void testMarkov()
{
    // A fully connected cyclic source: 0 -> 2 -> 4 -> 2 -> 0 ...
    int8_t source[64];
    const int motif[4] = { 0, 2, 4, 2 };
    for (int i = 0; i < 64; i++)
        source[i] = (int8_t)motif[i % 4];

    for (int order = 1; order <= 4; order++)
    {
        for (uint32_t seed : { 1u, 2u, 99u, 0xABCDEFu })
        {
            int8_t out[96];
            int n = mtGen::markovGenerate(
              source, 64, order, 96, 0, 127, 0, kChromatic, seed, out);
            CHECK(n == 96);
            for (int i = 0; i < n; i++)
            {
                CHECK(out[i] >= 0 && out[i] <= 127);
            }

            // Every emitted window of order+1 notes must exist in the source:
            // the generator only emits learned transitions.
            for (int i = 0; i + order < n; i++)
            {
                int window[mtGen::kMaxMarkovOrder + 1];
                for (int k = 0; k < order + 1; k++)
                    window[k] = out[i + k];
                CHECK(containsSubsequence(source, 64, window, order + 1));
            }

            // Deterministic for the same seed.
            int8_t out2[96];
            int n2 = mtGen::markovGenerate(
              source, 64, order, 96, 0, 127, 0, kChromatic, seed, out2);
            CHECK(n2 == n);
            CHECK(std::memcmp(out, out2, (size_t)n) == 0);
        }
    }

    // Scale lock and range are respected.
    const uint16_t major = scaleDef[2];
    int8_t out[64];
    int n = mtGen::markovGenerate(source, 64, 2, 64, 0, 127, 0, major, 5, out);
    CHECK(n == 64);
    for (int i = 0; i < n; i++)
        CHECK(mtGen::inScale(out[i], 0, major));

    // Source notes outside the requested range -> no usable material.
    n = mtGen::markovGenerate(source, 64, 1, 32, 90, 100, 0, kChromatic, 6, out);
    CHECK(n == 0);

    // Empty source and all-rest source.
    CHECK(mtGen::markovGenerate(nullptr, 0, 1, 32, 0, 127, 0, kChromatic, 1, out) == 0);
    int8_t rests[8] = { -1, -2, -1, -3, -1, -4, -1, -2 };
    CHECK(mtGen::markovGenerate(rests, 8, 1, 32, 0, 127, 0, kChromatic, 1, out) == 0);

    // Single-note source still produces notes (restart/extension path).
    int8_t single[1] = { 60 };
    n = mtGen::markovGenerate(single, 1, 1, 8, 60, 60, 0, kChromatic, 3, out);
    CHECK(n == 8);
    for (int i = 0; i < n; i++)
        CHECK(out[i] == 60);
}

// Mirrors Sequencer::isEuclideanStep (core/source/mtSequencer_patternEdit.cpp).
static bool euclideanSelected(int offset, int density, int selectionLength)
{
    return mtGen::euclideanHit(offset, density, selectionLength, 0);
}

static void testStepSelectors()
{
    const int length = 16;
    const int density = 4;

    // Euclidean selector combined with Walk.
    mtGen::WalkState walk;
    mtGen::walkInit(walk, 42);
    int walkNotes[16];
    int selected = 0;
    for (int offset = 0; offset < length; offset++)
    {
        if (!euclideanSelected(offset, density, length))
            continue;
        walkNotes[selected++] = mtGen::walkNext(walk, 48, 72, 3, 0, scaleDef[2]);
    }
    CHECK(selected == 4);
    for (int i = 0; i < selected; i++)
    {
        CHECK(walkNotes[i] >= 48 && walkNotes[i] <= 72);
        CHECK(mtGen::inScale(walkNotes[i], 0, scaleDef[2]));
    }

    // Euclidean selector combined with Turing (register clocks only on
    // selected steps, so exactly `selected` notes are emitted).
    mtGen::TuringState turing;
    mtGen::turingSeed(turing, 8, 42);
    int turingNotes[16];
    int turingSelected = 0;
    for (int offset = 0; offset < length; offset++)
    {
        if (!euclideanSelected(offset, density, length))
            continue;
        turingNotes[turingSelected++] = mtGen::turingNext(turing, 48, 72, 100, 0, scaleDef[2]);
    }
    CHECK(turingSelected == 4);
    for (int i = 0; i < turingSelected; i++)
        CHECK(turingNotes[i] >= 48 && turingNotes[i] <= 72);

    // Random selector (mirrors isStepToFillNote's Random branch) combined with
    // Walk; roughly half of the steps are filled and all notes stay valid.
    mtGen::Rng selector(123);
    mtGen::WalkState randomWalk;
    mtGen::walkInit(randomWalk, 123);
    int randomSelected = 0;
    for (int offset = 0; offset < 128; offset++)
    {
        if (selector.range(0, 127) < 64)
        {
            randomSelected++;
            int note = mtGen::walkNext(randomWalk, 0, 127, 5, 0, kChromatic);
            CHECK(note >= 0 && note <= 127);
        }
    }
    CHECK(randomSelected > 32 && randomSelected < 96);
}

static int countEuclideanHits(int length, int density, int rotate)
{
    int hits = 0;
    for (int offset = 0; offset < length; offset++)
    {
        if (mtGen::euclideanHit(offset, density, length, rotate))
            hits++;
    }
    return hits;
}

static void testEuclidean()
{
    // Classic E(3, 8) pattern.
    const int expected[3] = { 0, 3, 6 };
    int hitIndex = 0;
    for (int offset = 0; offset < 8; offset++)
    {
        bool hit = mtGen::euclideanHit(offset, 3, 8, 0);
        if (hit)
        {
            CHECK(hitIndex < 3);
            CHECK(expected[hitIndex] == offset);
            hitIndex++;
        }
    }
    CHECK(hitIndex == 3);

    // Exactly `density` hits for every density that fits the selection, at
    // every rotation, and the hit count does not depend on rotation.
    for (int length = 1; length <= 32; length++)
    {
        for (int density = 1; density <= length; density++)
        {
            for (int rotate : { 0, 1, length - 1, length, length + 5, -1 })
            {
                CHECK(countEuclideanHits(length, density, rotate) == density);
            }
        }
    }

    // Oversized Events clamp to the selection length (one hit per step), they
    // never create more hits than there are steps.
    for (int length = 1; length <= 32; length++)
    {
        for (int density : { length, length + 1, length * 4, 128, 1000 })
        {
            CHECK(countEuclideanHits(length, density, 0) == length);
        }
    }

    // No events below density 1 or for an empty selection.
    CHECK(countEuclideanHits(16, 0, 0) == 0);
    CHECK(countEuclideanHits(16, -3, 0) == 0);
    for (int offset = 0; offset < 8; offset++)
    {
        CHECK(!mtGen::euclideanHit(offset, 4, 0, 0));
        CHECK(!mtGen::euclideanHit(offset, 4, -8, 0));
    }

    // Rotate shifts the pattern: offset with rotate r is the unrotated hit at
    // offset + r (mod length).
    for (int length = 2; length <= 24; length++)
    {
        for (int density = 1; density <= length; density++)
        {
            for (int rotate = 0; rotate < length; rotate++)
            {
                for (int offset = 0; offset < length; offset++)
                {
                    CHECK(mtGen::euclideanHit(offset, density, length, rotate) ==
                          mtGen::euclideanHit((offset + rotate) % length, density, length, 0));
                }
            }
        }
    }

    // Negative rotation wraps instead of shifting outside the pattern.
    for (int rotate = -24; rotate < 0; rotate++)
    {
        for (int offset = 0; offset < 16; offset++)
        {
            CHECK(mtGen::euclideanHit(offset, 5, 16, rotate) ==
                  mtGen::euclideanHit(offset, 5, 16, rotate + 16));
        }
    }
}

static void testInstrumentPick()
{
    const uint8_t samples[4] = { 0, 1, 47, 0 };
    const uint8_t midi[4] = { 0, 49, 50, 0 };
    const uint8_t synth[4] = { 0, 64, 65, 0 };
    const uint8_t mixture[6] = { 0, 64, 48, 49, 65, 0 };
    const uint8_t synthThenMidi[4] = { 64, 49, 0, 0 };

    // No usable instrument on the track -> old fallback.
    CHECK(mtGen::pickTrackFillInstrument(samples, 4, 48) == 48);

    // Last MIDI instrument on the track.
    CHECK(mtGen::pickTrackFillInstrument(midi, 4, 48) == 50);

    // Last synth instrument on the track.
    CHECK(mtGen::pickTrackFillInstrument(synth, 4, 48) == 65);

    // Synth instruments beat MIDI instruments, whichever comes first.
    CHECK(mtGen::pickTrackFillInstrument(mixture, 6, 48) == 65);
    CHECK(mtGen::pickTrackFillInstrument(synthThenMidi, 4, 48) == 64);

    CHECK(mtGen::pickTrackFillInstrument(nullptr, 4, 51) == 51);
    CHECK(mtGen::pickTrackFillInstrument(midi, 0, 52) == 52);
    CHECK(mtGen::pickTrackFillInstrument(midi, -1, 53) == 53);
}

static void testFxBounds()
{
    CHECK(mtGen::chanceFromPercent(-10) == 0);
    CHECK(mtGen::chanceFromPercent(0) == 0);
    CHECK(mtGen::chanceFromPercent(50) == 50);
    CHECK(mtGen::chanceFromPercent(100) == 100);
    CHECK(mtGen::chanceFromPercent(1000) == 100);

    for (int count = -5; count <= 20; count++)
    {
        int period = mtGen::rollPeriodFromCount(count);
        CHECK(period >= mtGen::kRollPeriodMin);
        CHECK(period <= mtGen::kRollPeriodMax);
    }
    CHECK(mtGen::rollPeriodFromCount(0) == 1);
    CHECK(mtGen::rollPeriodFromCount(4) == 4);
    CHECK(mtGen::rollPeriodFromCount(99) == 8);
}

int main()
{
    testScales();
    testWalk();
    testTuring();
    testMarkov();
    testStepSelectors();
    testEuclidean();
    testInstrumentPick();
    testFxBounds();

    std::printf("%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
