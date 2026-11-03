#ifndef SOURCE_MT_GENERATORS_H_
#define SOURCE_MT_GENERATORS_H_

// Pure, hardware independent generative fill algorithms.
//
// Everything in this file is plain C++: no project globals, no hardware, no
// file formats.  Every entry point is deterministic for a given seed (and, for
// the Turing generator, for a given register state).  The sequencer adapters
// in mtSequencer_patternEdit.cpp are the only code that touches pattern data.

#include <stdint.h>

namespace mtGen
{

static constexpr int kNoteMin = 0;
static constexpr int kNoteMax = 127;

static constexpr int kMaxRegisterLength = 16; // Turing shift register bits
static constexpr int kMaxMarkovOrder = 4;
static constexpr int kMaxSteps = 128; // pattern steps are 0..127

// Ranges copied from Sequencer::strFxConsts (see mtSequencer.h).  The adapter
// static_asserts that these still match the source.
static constexpr int kChanceProbMax = 100;
static constexpr int kRollPeriodMin = 1;
static constexpr int kRollPeriodMax = 8;

//----------------------------------------------------------------------------
// scales
//----------------------------------------------------------------------------
// scaleMask is the 12 bit scaleDef[] entry: bit n is set when the semitone
// offset n above the root belongs to the scale.
bool inScale(int note, uint8_t root, uint16_t scaleMask);

// Nearest note to `note` that is inside [lo, hi] and in the scale.
// Returns lo when the range contains no scale note.
int nearestInScale(int note, int lo, int hi, uint8_t root, uint16_t scaleMask);

// Number of scale notes inside [lo, hi].
int countInScale(int lo, int hi, uint8_t root, uint16_t scaleMask);

//----------------------------------------------------------------------------
// deterministic random source (xorshift32)
//----------------------------------------------------------------------------
struct Rng
{
    uint32_t state = 0x9E3779B9u;

    Rng() = default;
    explicit Rng(uint32_t seed) { reseed(seed); }

    void reseed(uint32_t seed)
    {
        // xorshift32 locks up on zero
        state = (seed == 0) ? 0x9E3779B9u : seed;
    }

    uint32_t next()
    {
        uint32_t x = state;
        x ^= x << 13;
        x ^= x >> 17;
        x ^= x << 5;
        state = x;
        return x;
    }

    // inclusive bounds
    int range(int lo, int hi)
    {
        if (hi <= lo)
            return lo;
        return lo + (int)(next() % (uint32_t)(hi - lo + 1));
    }
};

//----------------------------------------------------------------------------
// 1. Walk - scale quantised random walk
//----------------------------------------------------------------------------
struct WalkState
{
    Rng rng;
    int note = 0;
    bool started = false;
};

void walkInit(WalkState& state, uint32_t seed);

// Next note of the walk.  `maxJump` is the maximum absolute semitone move
// (after scale quantisation).  The first call picks a random in-scale note
// inside [lo, hi].  Every returned note is inside [lo, hi] and in the scale.
int walkNext(WalkState& state,
             int lo,
             int hi,
             int maxJump,
             uint8_t root,
             uint16_t scaleMask);

//----------------------------------------------------------------------------
// 2. Turing - shift register melody
//----------------------------------------------------------------------------
struct TuringState
{
    Rng rng;
    uint16_t reg = 0;
    uint8_t length = 8;
    bool seeded = false;
};

void turingReset(TuringState& state);

// Seeds the register from `seed`.  `length` is clamped to 1..16.
void turingSeed(TuringState& state, int length, uint32_t seed);

// Returns the note for the current register value, then clocks the register:
// it rotates one bit and, with probability (100 - lockPercent) per bit, flips
// that bit.  lockPercent is clamped to 0..100.  At 100 the register only
// rotates, so the output repeats every `length` calls.
int turingNext(TuringState& state,
               int lo,
               int hi,
               int lockPercent,
               uint8_t root,
               uint16_t scaleMask);

// Maps a register value to an in-scale note inside [lo, hi].
int turingNoteForRegister(uint16_t reg,
                          int lo,
                          int hi,
                          uint8_t root,
                          uint16_t scaleMask);

//----------------------------------------------------------------------------
// 3. Markov - learn note transitions from a source sequence
//----------------------------------------------------------------------------
// source notes < 0 (empty / note-off) are ignored.  The generated notes are
// always inside [lo, hi] and in the scale, and each note after the first
// `order` notes is an observed source transition unless the model has no
// in-range/in-scale successor for the current context (then the sequence
// restarts from a random source note).
// Returns the number of notes written to out (0 when there is no usable
// source material).
int markovGenerate(const int8_t* source,
                   int sourceLen,
                   int order,
                   int outCount,
                   int lo,
                   int hi,
                   uint8_t root,
                   uint16_t scaleMask,
                   uint32_t seed,
                   int8_t* out);

//----------------------------------------------------------------------------
// Prob / Ratchet value mapping (kept here so host tests can check the bounds)
//----------------------------------------------------------------------------
int chanceFromPercent(int percent); // 0..kChanceProbMax
int rollPeriodFromCount(int count); // kRollPeriodMin..kRollPeriodMax

} // namespace mtGen

#endif // SOURCE_MT_GENERATORS_H_
