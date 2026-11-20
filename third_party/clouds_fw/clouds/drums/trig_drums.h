// trig_drums.h - 8-voice trigger drum machine. Port of TrigDrumsEngine.h
// (VersionBeatMidiSendTrigs) to Clouds: 32 kHz, buffers in the shared
// arena, 16-bit echo (up to 0.5 s), fast math (no libm in the audio
// path), reverb at half rate. The jack mapping below is the Versio one; on
// Clouds see drum_mode.cc and README.md.
//
//
// VOICES (trigger input -> voice; the knob above the jack tunes it)
//   jack 1 (K0) BD   kick         jack 5 (K4) CP   clap
//   jack 2 (K1) SD   snare        jack 6 (K5) MT   multi drum:
//   jack 3 (K2) OH   open hat                      tom / conga / rim / cowbell
//                    (CH chokes)  jack 7 (K6) SA   chord stab (dub techno)
//   jack 4 (K3) CH   closed hat,  gate        SB   resonator chord (dub)
//                    its triggers also clock the pattern (see the firmware)
// Every jack is knob + CV summed: a fast jump (trigger / gate) plays the
// voice (its height = velocity), the slow part (knob + slow CV) is the
// voice's parameter. USB MIDI notes play the voices too.
//
// SW0 right: a pattern plays (locked to the beat: MIDI clock / closed-hat
// clock, the beat tracked from the audio, or the free tempo). A voice that
// gets external triggers drops out of the pattern for 4 s.
//
// SUBMODE  row (SW0) = left   TRIGS only, ANALOG kit (808-ish, round)
//                      mid    TRIGS only, PUNCH kit (909-ish, clicky)
//                      right  PATTERN plays (+ trigs), kit of the trigger
//                             row used last
//          double-tap: trigger rows = LO-FI grit on / off (12 bit /
//          24 kHz, on either kit), pattern row = next pattern (6)
//          col (SW1) = AUDIO IN
//                             THRU   input passes (50 %), drums on top
//                             SKIN   the input becomes the noise of SD, CP,
//                                    hats and MT; the kick ducks the input
//                             CHOP   input heard only through the hits: BD
//                                    opens the lows, SD the mids, hats the
//                                    highs, CP / MT / synths their bands
// KNOBS edit ONE voice at a time (the page; the firmware steps through the
// 8 voices with taps). The same layout on every page:
//   K0 TUNE   K1 DECAY   K2 CHARACTER A   K3 CHARACTER B
//   K4 FILTER  noon = open; CCW low-pass, CW high-pass, the resonance
//              rising as you move away from noon (peaks mid-way, eases off
//              at the ends so it never screams)
//   K5 DRIVE   CCW clean; first half warm saturation, second half
//              turns into a folding distortion (level-matched)
//   K6 SEND    reverb up to 3/4, the last quarter adds the echo
//   Every hit gets a small random pan around the centre (the kick stays
//   in the middle).
//   character A / B:  BD click / drive · SD snappy / noise tail ·
//   OH, CH brightness / noise · CP spread / width · MT bend / noise
//   (MT TUNE sweeps tom > conga > rim > cowbell) · SA CHORD / FILTER ·
//   SB CHORD / BRIGHTNESS. SA and SB TUNE = chord root, 1 V/oct-ish
//   (C2..C7). Chords (CHARACTER A): m · m7 · m9 · m11 · m7sus4 ·
//   m(add9) · maj7 · open fifths. SA: detuned saws through a resonant
//   low-pass with a filter envelope (CHARACTER B: dark & soft -> bright &
//   resonant). SB: a noise burst rings 4-5 comb resonators tuned to the
//   chord (DECAY = ring time). Both send to the echo + reverb by default.
// SEND bus: tempo-synced ping-pong echo (dotted 8th) -> reverb.
// DUB (button held, momentary, smooth): every decay 20 % longer and all
// voices into the reverb (its tail also grows); fades out over 1.5 s.
// USB MIDI: GM drum notes
// play the voices, channel 2 / 3 notes play SA / SB at that pitch, MIDI
// clock sets the tempo
#pragma once
#include <string.h>
#include "clouds/bender/pn.h"
#include "clouds/bender/arena.h"
#include "clouds/drums/beat_tracker.h"

namespace tdr
{

enum Voice
{
    BD,
    SD,
    CH,
    OH,
    CP,
    MT,
    SA,
    SB,
    kV
};

// trigger requests (velocity 0..1), from the jacks / gate (audio thread)
// and USB MIDI (main loop); consumed at the start of each block
static volatile float req[kV];
static volatile bool  reqExt[kV];          // came from outside (mutes pattern)
static volatile float midiNote[2] = {-1.f, -1.f}; // SA / SB pitch from MIDI

// voice parameters, written by the firmware's knob pages:
//   [voice][0 TUNE, 1 DECAY, 2 CHAR A, 3 CHAR B, 4 FILTER, 5 DRIVE, 6 SEND]
enum Param
{
    P_TUNE,
    P_DECAY,
    P_A,
    P_B,
    P_FILT,
    P_DRIVE,
    P_SEND,
    P_SHAPE, // CONTOUR shape: < 0.5 tight (tail gated), > 0.5 soft attack
    P_LEVEL, // 0.7 = unity, 1 = +6 dB, 0 = off
    kP
};
static float prm[8][kP];
static const float kPrmDef[8][kP] = {
    {0.5f, 0.45f, 0.5f, 0.45f, 0.5f, 0.1f, 0.f, 0.5f, 0.7f},   // BD
    {0.5f, 0.4f, 0.5f, 0.4f, 0.5f, 0.1f, 0.f, 0.5f, 0.7f},     // SD
    {0.5f, 0.4f, 0.5f, 0.5f, 0.5f, 0.f, 0.f, 0.5f, 0.7f},      // CH
    {0.5f, 0.35f, 0.5f, 0.5f, 0.5f, 0.f, 0.f, 0.5f, 0.7f},     // OH
    {0.5f, 0.4f, 0.5f, 0.5f, 0.5f, 0.1f, 0.2f, 0.5f, 0.7f},    // CP
    {0.3f, 0.4f, 0.5f, 0.5f, 0.5f, 0.1f, 0.f, 0.5f, 0.7f},     // MT
    {0.35f, 0.4f, 0.35f, 0.45f, 0.5f, 0.15f, 0.82f, 0.5f, 0.7f}, // SA: A3 m9 stab, echo
    {0.15f, 0.5f, 0.2f, 0.5f, 0.5f, 0.f, 0.78f, 0.5f, 0.7f},     // SB: A2 m7 resonator
};
// globals of the LOAD/SAVE layer (knob 0..1; defaults = the Versio sound)
static volatile float panWidth  = 0.3333f; // random pan per hit 0..0.9
static volatile float rvTail    = 0.667f; // reverb comb feedback 0.6..0.9
static volatile float echoFb    = 0.56f;  // echo feedback 0..0.75
static volatile bool dubReq = false; // button held (firmware)
static float         dub    = 0.f;   // smoothed 0..1
// momentary actions (firmware): ECHO THROW (everything into the echo,
// longer feedback) and MUTE (the dry drums drop out, the tails ring on)
static volatile bool throwReq = false, muteReq = false;
static float         thr = 0.f, mute = 0.f; // smoothed 0..1
static volatile int  page   = 0;     // voice the knobs edit (for the LEDs)

// small dub reverb: 4 damped combs + 2 allpasses per side (Freeverb-ish)
static constexpr int kCombs = 4, kAps = 2;
// lengths: the Versio ones x 2/3 (48 -> 32 kHz), same reverb times
// the reverb runs at half rate (16 kHz): the same delay times as the
// Versio engine at half the length
static const int     kCombLen[kCombs] = {563, 587, 541, 523};
static const int     kApLen[kAps]     = {199, 163};
static constexpr int kCombMax = 600, kApMax = 216; // + 11 for the right side
typedef float CombLine[kCombMax];
typedef float ApLine[kApMax];
static CombLine* rvComb[2] = {nullptr, nullptr}; // [side][comb], main arena
static ApLine*   rvAp[2]   = {nullptr, nullptr};
static int                 rvCi[2][kCombs], rvAi[2][kAps];
static float               rvLp[2][kCombs], rvInLp = 0.f;
static float               rvAcc = 0.f, rvLast[2] = {0, 0};
static bool                rvOdd = false;
static volatile bool       lite  = false; // CPU guard: reverb pauses and
                                          // quiet tails (< -50 dB) are cut
static float               ampFloor = 1e-4f;

static constexpr float kTwoPi = 6.2831853f;

// voice levels into the bus, set from measured peaks so a single hit
// peaks at about: BD 0.55, SD 0.45, hats 0.3, CP / MT 0.35, synths 0.3,
// leaving headroom for hits that land together
static constexpr float kGainBd = 0.53f, kGainSd = 0.36f, kGainCp = 0.25f;
static constexpr float kGainMt = 0.23f, kGainSa = 0.5f, kGainSb = 0.6f;

// ------------------------------------------------------------- patterns ---
// digit = velocity (9 = accent), '.' = rest. Order: BD SD CH OH CP MT SA SB
static constexpr int kPats = 7;
static const char* const kPat[kPats][kV] = {
    // HOUSE
    {"9...9...9...9...", "................", "...3...3...3...3", "..7...7...7...7.",
     "....9.......9...", "......5.......5.", "..........7.....", "7..5..7...5..7.."},
    // TECHNO
    {"9...9...9...9...", "................", "5.3.5.3.5.3.5.3.", "..7...7...7...7.",
     "....7.......7...", "...4..4....4..4.", "......5.......5.", "9.5.9.5.9.5.9.5."},
    // BOOM BAP
    {"9......5.9......", "....9.......9...", "7.5.7.5.7.5.7.5.", "..............5.",
     "............5...", "...........3....", "7...........5...", "9......5.9......"},
    // BREAK
    {"9.........9.5...", "....9..3.5..9..5", "7575757575757575", "......5.........",
     "................", "..3.....3....3..", "....5.....5.....", "9.........9.5..."},
    // ELECTRO
    {"9.....9...9..5..", "....9.......9...", "3535353535353535", "................",
     "....5.......5...", "..5...5...5...5.", "9..5..9..5..9.5.", "9.....9...9..5.."},
    // PERC
    {"9.....5...9.....", "......5.......5.", "5.55.5.55.5.55.5", "................",
     "....7.......7...", "7.5.5.7..57.5.5.", "..5....5..5....5", "9..5....9..5...."},
    // DUB TECHNO: off-beat stab, a ringing chord, soft hats
    {"9...9...9...9...", "................", "..4...4...4...4.", "................",
     "................", "................", "..9.......7.....", "......7........."},
};
// synth note offsets per step (semitones)
static const int8_t kNoteA[kPats][16] = {
    {0, 0, 7, 0, 12, 0, 7, 0, 0, 0, 10, 0, 7, 0, 5, 0},
    {0, 0, 0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 12, 0},
    {12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 7, 0, 0, 0},
    {0, 0, 0, 0, 7, 0, 0, 0, 0, 0, 3, 0, 0, 0, 0, 0},
    {0, 0, 0, 12, 0, 0, 7, 0, 0, 10, 0, 0, 7, 0, 3, 0},
    {0, 0, 7, 0, 0, 0, 0, 5, 0, 0, 12, 0, 0, 0, 0, 7},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};
static const int8_t kNoteB[kPats][16] = {
    {0, 0, 0, 12, 0, 0, 10, 0, 0, 0, 0, 7, 0, 0, 5, 0},
    {0, 0, 12, 0, 0, 0, 12, 0, 0, 0, 12, 0, 0, 0, 12, 0},
    {0, 0, 0, 0, 0, 0, 0, -2, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 5, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, -2, 0, 0, 3, 0, 0},
    {0, 0, 0, 7, 0, 0, 0, 0, 0, 0, 0, 5, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

// chords (semitones above the root), mostly minor; -99 = unused
static constexpr int kChordMax = 5;
static const int8_t  kChord[8][kChordMax] = {
    {0, 3, 7, 12, -99},  // m
    {0, 3, 7, 10, -99},  // m7
    {0, 3, 7, 10, 14},   // m9
    {0, 3, 7, 10, 17},   // m11
    {0, 5, 7, 10, -99},  // m7sus4
    {0, 7, 14, 15, -99}, // m(add9), open voicing
    {0, 4, 7, 11, -99},  // maj7
    {0, 7, 12, 19, -99}, // open fifths
};

// ---------------------------------------------------------------- state ---
static float   sr = 32000.f, isr = 1.f / 32000.f;
static pn::Rng rng;

inline float Coef(float sec) { return pn::Exp2(-1.4426950f / (sec * sr)); }
inline float Sq(float ph) { return ph < 0.5f ? 1.f : -1.f; }
inline float Wrap(float p) { return p >= 1.f ? p - 1.f : p; }

// small TPT state-variable filter (band / low / high)
struct Svf2
{
    float ic1 = 0.f, ic2 = 0.f, g = 0.1f, k = 1.f, a1 = 0.9f, ga1 = 0.09f;
    void  Reset() { ic1 = ic2 = 0.f; }
    void  Set(float hz, float q)
    {
        g   = pn::Tan(3.1415927f * pn::Min(hz, sr * 0.45f) * isr);
        k   = 1.f / q;
        a1  = 1.f / (1.f + g * (g + k));
        ga1 = g * a1;
    }
    // returns band, writes low / high
    float Run(float x, float& lo, float& hi)
    {
        const float v3 = x - ic2;
        const float v1 = a1 * ic1 + ga1 * v3;
        const float v2 = ic2 + g * v1;
        ic1            = 2.f * v1 - ic1;
        ic2            = 2.f * v2 - ic2;
        lo             = v2;
        hi             = x - k * v1 - v2;
        return v1;
    }
};

struct Bd
{
    float ph = 0.f, pe = 0.f, amp = 0.f, cl = 0.f;
} bd;
struct Sd
{
    float p1 = 0.f, p2 = 0.f, pe = 0.f, body = 0.f, nz = 0.f, lp = 0.f;
} sd;
struct Hat
{
    float ph[6] = {0, 0.13f, 0.37f, 0.61f, 0.29f, 0.83f};
    float amp = 0.f, h1 = 0.f, h2 = 0.f, x1 = 0.f, x2 = 0.f, choke = 1.f;
} ch, oh;
struct Cp
{
    float    b = 0.f, tail = 0.f, vel = 1.f;
    uint32_t t = 99999;
    Svf2     bp;
} cp;
struct Mt
{
    float ph = 0.f, ph2 = 0.f, pe = 0.f, amp = 0.f, fast = 0.f, slap = 0.f;
    int   type = 0;
    bool  ping = false;
    Svf2  bp;
} mt;
// SA chord stab: two detuned band-limited saws per chord note
struct Stab
{
    float ph[kChordMax][2];
    float inc[kChordMax][2];
    int   n = 4;
    float amp = 0.f, fenv = 0.f, vel = 1.f;
    Svf2  lp;
} sa;
// SB resonator chord: a noise burst rings one comb per chord note
static constexpr uint32_t kRc = 1024, kRcM = kRc - 1; // 31 Hz at 32 kHz
typedef float RcLine[kRc];
static RcLine* rcBuf = nullptr; // [chord note], CCM
static float   sbPeak = 0.f;     // per block: the resonator sleeps below -80 dB
struct Reso
{
    uint32_t w = 0;
    float    len[kChordMax];
    float    lp[kChordMax];
    int      n = 4, exc = 0;
    float    amp = 0.f, nlp = 0.f;
    uint32_t act = 0; // samples left to run
} sb;

// SEND bus echo: tempo-synced ping-pong (dotted 8th), feeds the reverb
static constexpr uint32_t kEch = 1u << 14, kEchM = kEch - 1; // 0.51 s
static int16_t* ech[2] = {nullptr, nullptr}; // L main arena, R CCM
static uint32_t            ew = 0;
static float               eLp[2] = {0, 0}, eHp[2] = {0, 0}, eDs = 18000.f;

// band-limited saw (polyBLEP)
inline float Blep(float t, float dt)
{
    if(t < dt)
    {
        t /= dt;
        return t + t - t * t - 1.f;
    }
    if(t > 1.f - dt)
    {
        t = (t - 1.f) / dt;
        return t * t + t + t + 1.f;
    }
    return 0.f;
}
inline int ChordOf(float a)
{
    int z = (int)(a * 7.999f);
    return z < 0 ? 0 : (z > 7 ? 7 : z);
}

// per-voice FILTER (TPT state-variable, low-pass or high-pass) and the
// random pan of the current hit
static Svf2  vf[kV];
static float hitPan[kV];
// CONTOUR: samples since the hit, the tail gate gain
static uint32_t ctN[kV];
static float    ctF[kV];

// DRIVE: saturation that turns into a folding distortion, level-matched
// at the peak of a typical hit (0.5)
// soft saturation without a divide (cheap on the M4): cubic, flat at +-1.5
inline float Sat(float z)
{
    z = pn::Clamp(z, -1.5f, 1.5f);
    return z - 0.148148f * z * z * z;
}
inline float Drive(float x, float pre, float m, float comp)
{
    const float z   = x * pre;
    const float sat = Sat(z);
    if(m <= 0.f)
        return sat * comp;
    // distortion: the saturated signal folds over more and more
    const float fold = pn::Sin(sat * (1.6f + 2.4f * m));
    return (sat + (fold - sat) * m) * comp;
}

// pattern / external triggers
static int      lastStep = -1, pat = 0;
static bool     lastA = false;
static int      kitBase = 0;     // 0 ANALOG, 1 PUNCH
static bool     grit    = false; // LO-FI grit on top (double-tap, trig rows)
static uint32_t extHold[kV]; // samples left during which the pattern is muted
// input processing
static float inEnv = 0.f, lo1 = 0.f, lo2 = 0.f, hiLp = 0.f;
static Svf2  inMid, inCp;
// grit
static float holdPh = 0.f, held[2] = {0, 0}, gLp[2] = {0, 0};

static volatile float hitShow[kV];
static volatile int   showPat = 0;
static volatile bool  showRun = true;

// buffers from the arena (zeroed there); call when the mode starts
inline void Attach()
{
    for(int c = 0; c < 2; c++)
    {
        rvComb[c] = arena::M.Take<CombLine>(kCombs);
        rvAp[c]   = arena::M.Take<ApLine>(kAps);
    }
    ech[0] = arena::M.Take<int16_t>(kEch);
    ech[1] = arena::C.Take<int16_t>(kEch);
    rcBuf  = arena::C.Take<RcLine>(kChordMax);
}
inline void Clear()
{
    sb.act = 0;
    memset(rvCi, 0, sizeof(rvCi));
    memset(rvAi, 0, sizeof(rvAi));
    memset(rvLp, 0, sizeof(rvLp));
    rvAcc = rvInLp = 0.f, rvLast[0] = rvLast[1] = 0.f;
    bd.amp = sd.body = sd.nz = ch.amp = oh.amp = cp.b = cp.tail = 0.f;
    mt.amp = sa.amp = sb.amp = 0.f;
    for(int v = 0; v < kV; v++)
        req[v] = 0.f, extHold[v] = 0;
}
// a NaN / runaway reached the engine: silence every state that feeds back
// (reverb, echo, resonators, filters), so the next block starts clean
inline void Panic()
{
    for(int c = 0; c < 2; c++)
    {
        memset(rvComb[c], 0, sizeof(CombLine) * kCombs);
        memset(rvAp[c], 0, sizeof(ApLine) * kAps);
        memset(ech[c], 0, sizeof(int16_t) * kEch);
        eLp[c] = eHp[c] = 0.f;
        held[c] = gLp[c] = 0.f;
    }
    memset(rcBuf, 0, sizeof(RcLine) * kChordMax);
    for(int j = 0; j < kChordMax; j++)
        sb.lp[j] = 0.f;
    sb.nlp = 0.f;
    eDs    = 18000.f;
    for(int v = 0; v < kV; v++)
        vf[v].Reset(), ctF[v] = 1.f;
    cp.bp.Reset(), mt.bp.Reset(), sa.lp.Reset(), inMid.Reset(), inCp.Reset();
    sd.lp = ch.h1 = ch.h2 = oh.h1 = oh.h2 = 0.f;
    inEnv = lo1 = lo2 = hiLp = 0.f;
    dub   = 0.f;
    Clear();
}

inline void Init(float s)
{
    sr  = s;
    isr = 1.f / s;
    memcpy(prm, kPrmDef, sizeof(prm));
}

// one stereo reverb sample; fb = comb feedback (tail length)
inline void Reverb(float x, float fb, float& l, float& r)
{
    rvInLp += (x - rvInLp) * 0.45f; // darker input (~6 kHz)
    rvAcc += rvInLp;
    rvOdd = !rvOdd;
    if(!rvOdd)
    {
        // even sample: hold, odd sample: compute (linear interpolation)
        l = rvLast[0];
        r = rvLast[1];
        return;
    }
    const float xin = rvAcc * 0.5f;
    rvAcc           = 0.f;
    if(lite)
    {
        l = rvLast[0] *= 0.99f;
        r = rvLast[1] *= 0.99f;
        return;
    }
    float o[2];
    for(int c = 0; c < 2; c++)
    {
        const int sp = c ? 11 : 0; // right side a little longer
        float     s  = 0.f;
        for(int k = 0; k < kCombs; k++)
        {
            float* b   = rvComb[c][k];
            const int L = kCombLen[k] + sp;
            int& i      = rvCi[c][k];
            const float y = b[i];
            rvLp[c][k] += (y - rvLp[c][k]) * 0.84f; // damping (0.6 at 32 kHz)
            b[i] = xin * 0.1f + rvLp[c][k] * fb;
            if(++i >= L)
                i = 0;
            s += y;
        }
        for(int k = 0; k < kAps; k++)
        {
            float* b   = rvAp[c][k];
            const int L = kApLen[k] + sp;
            int& i      = rvAi[c][k];
            const float bo = b[i];
            b[i]           = s + bo * 0.5f;
            s              = bo - s;
            if(++i >= L)
                i = 0;
        }
        o[c] = s;
    }
    l         = 0.5f * (rvLast[0] + o[0]);
    r         = 0.5f * (rvLast[1] + o[1]);
    rvLast[0] = o[0];
    rvLast[1] = o[1];
}
inline void Activate(const Panel& p)
{
    lastA    = p.a;
    lastStep = -1;
}

// ----------------------------------------------------------------- hits ---
inline void Hit(int v, float vel, float noteOff)
{
    hitShow[v] = pn::Max(hitShow[v], vel);
    ctN[v]     = 0;
    ctF[v]     = 1.f;
    // a small random place around the centre for every hit (kick: centre)
    hitPan[v] = v == BD ? 0.f : 0.9f * panWidth * rng.B();
    switch(v)
    {
        case BD:
            bd.ph = 0.f, bd.pe = 1.f, bd.amp = vel, bd.cl = vel;
            break;
        case SD:
            sd.p1 = sd.p2 = 0.f, sd.pe = 1.f, sd.body = vel, sd.nz = vel;
            break;
        case CH:
            ch.amp = vel, ch.choke = 1.f;
            oh.choke = 0.f; // closed hat chokes the open one
            break;
        case OH:
            oh.amp = vel, oh.choke = 1.f;
            break;
        case CP:
            cp.t = 0, cp.b = vel, cp.tail = 0.f, cp.vel = vel;
            break;
        case MT:
            mt.ph = mt.ph2 = 0.f, mt.pe = 1.f, mt.amp = vel, mt.fast = vel,
            mt.slap = vel, mt.ping = true;
            break;
        case SA: // noteOff carries the root frequency
        {
            const int8_t* ch = kChord[ChordOf(prm[SA][P_A])];
            int           n  = 0;
            for(int j = 0; j < kChordMax && ch[j] > -99; j++, n++)
            {
                const float f = noteOff * pn::Exp2((float)ch[j] / 12.f);
                // the root: two saws +-7 cents; the other notes one saw,
                // detuned +-5 cents alternately (half the CPU of a pair each)
                sa.inc[j][0] = f * pn::Exp2((j == 0 ? -0.07f : ((j & 1) ? 0.05f : -0.05f)) / 12.f) * isr;
                sa.inc[j][1] = f * pn::Exp2(0.07f / 12.f) * isr;
                if(sa.amp < 1e-3f)
                    sa.ph[j][0] = rng.F(), sa.ph[j][1] = rng.F();
            }
            sa.n = n, sa.amp = vel, sa.fenv = 1.f, sa.vel = vel;
            break;
        }
        case SB:
        {
            const int8_t* ch = kChord[ChordOf(prm[SB][P_A])];
            int           n  = 0;
            for(int j = 0; j < kChordMax && ch[j] > -99; j++, n++)
                sb.len[j] = pn::Clamp(sr / (noteOff * pn::Exp2((float)ch[j] / 12.f)),
                                      16.f, (float)(kRc - 8));
            sb.n = n, sb.amp = vel, sb.exc = (int)(0.004f * sr);
            sb.act = (uint32_t)(8.f * sr); // longest ring, then it sleeps
            break;
        }
    }
}

// ------------------------------------------------------------- process ----
// k = layer 1 (tune + character), s = layer 2 (decay + second); in = the
// raw audio input (not the send), out = finished mix (dry included)
inline void Process(const Panel&       p,
                    const float* const* in,
                    float**             out,
                    size_t              size)
{
    // SW0: left = TRIGS + ANALOG kit, mid = TRIGS + PUNCH kit, right =
    // PATTERN (+ trigs) with the kit of the trigger row used last.
    // double-tap: pattern row = next pattern, trigger rows = LO-FI grit
    const int row = p.row, aud = p.col;
    if(row < 2)
        kitBase = row;
    if(p.a != lastA)
    {
        lastA = p.a;
        if(row == 2)
            pat = (pat + 1) % kPats;
        else
            grit = !grit;
    }
    const bool run = row == 2;
    ampFloor       = lite ? 3e-3f : 1e-4f;
    showPat = pat;
    showRun = run;

    // ---- THROW / MUTE: in 20 ms, out 0.3 s
    {
        const float st = (float)size / (0.02f * sr), sl = (float)size / (0.3f * sr);
        thr += pn::Clamp((throwReq ? 1.f : 0.f) - thr, -sl, st);
        mute += pn::Clamp((muteReq ? 1.f : 0.f) - mute, -st * 0.5f, st);
    }
    // ---- DUB: smooth in (0.15 s) and out (1.5 s); decays +20 %
    {
        const float tgt = dubReq ? 1.f : 0.f;
        const float st  = (float)size / ((dubReq ? 0.15f : 1.5f) * sr);
        dub += pn::Clamp(tgt - dub, -st, st);
    }
    const float dl = 1.f + 0.2f * dub;

    // ---- kit character
    const bool  punch    = kitBase == 1;
    const float bdBend   = punch ? 3.2f : 2.2f;
    const float bdPc     = Coef(punch ? 0.008f : 0.02f);
    const float hatNoise = punch ? 0.45f : 0.08f;
    const float sdHpHz   = punch ? 1800.f : 900.f;
    const float busDrive = grit ? 2.2f : (punch ? 1.4f : 1.f);
    // hats: the analog metal is mostly removed by the high-pass, so it
    // gets more gain than the noisier PUNCH hats
    const float hatG = punch ? 1.2f : 3.f;

    // ---- per-voice mix: random pan, FILTER, DRIVE, SEND (+ DUB)
    float gL[kV], gR[kV], snd[kV], esnd[kV];
    float dPre[kV], dM[kV], dComp[kV];
    bool  fOn[kV], fHp[kV];
    float fGain[kV];
    for(int v = 0; v < kV; v++)
    {
        const float pan = hitPan[v];
        gL[v]           = pn::Min(1.f, 1.f - pan);
        gR[v]           = pn::Min(1.f, 1.f + pan);
        // SEND: reverb over the first 3/4, the echo comes in on the last
        // quarter. DUB throws into both (the kick only a little: a thrown
        // kick muddies the echo and eats the headroom)
        const float k   = prm[v][P_SEND];
        const float dt  = dub * (v == BD ? 0.18f : 0.6f);
        snd[v]          = pn::Clamp(k / 0.75f + dt, 0.f, 1.f);
        esnd[v]         = pn::Clamp((k - 0.7f) / 0.3f, 0.f, 1.f) * 0.8f + dt
                  + thr * (v == BD ? 0.3f : 0.9f);
        // FILTER: distance from noon = how far it closes, resonance peaks
        // mid-way and eases off towards the end
        const float fd = prm[v][P_FILT] - 0.5f;
        const float t  = pn::Min(fabsf(fd) * 2.f, 1.f);
        fOn[v]         = t > 0.03f;
        fHp[v]         = fd > 0.f;
        if(fOn[v])
        {
            const float q  = 0.7f + 7.f * t * (1.f - t) + 1.5f * t;
            const float hz = fHp[v] ? 30.f * pn::Exp2(t * 8.5f)      // up to ~11 kHz
                                    : 18000.f * pn::Exp2(-t * 8.f);  // down to ~70 Hz
            vf[v].Set(hz, q);
            fGain[v] = 1.f / (1.f + 0.15f * (q - 0.7f));
        }
        // DRIVE: first half saturation, second half folding distortion
        const float u = prm[v][P_DRIVE];
        dPre[v]       = 1.f + 9.f * u * u;
        dM[v]         = pn::Clamp((u - 0.5f) * 2.f, 0.f, 1.f);
        dComp[v]      = u > 0.02f ? 0.5f / Sat(dPre[v] * 0.5f) : 0.f;
    }
    // the tail grows while dubbing
    const float rvFb = pn::Min(0.6f + 0.3f * rvTail + 0.12f * dub, 0.93f);
    // LEVEL per voice
    float lvl[kV];
    for(int v = 0; v < kV; v++)
    {
        const float l = prm[v][P_LEVEL] * (1.f / 0.7f);
        lvl[v]        = l * l;
    }

    // ---- CONTOUR shape per voice: CCW = tight (the tail is gated after
    // 15 ms .. 0.27 s), noon = natural, CW = soft attack (up to 30 ms)
    uint32_t    ctA[kV], ctH[kV];
    float       ctAi[kV];
    const float ctFc = Coef(0.006f);
    for(int v = 0; v < kV; v++)
    {
        const float sh = 2.f * prm[v][P_SHAPE] - 1.f;
        ctA[v]         = sh > 0.02f ? 1u + (uint32_t)(0.03f * sh * sh * sr) : 0u;
        ctAi[v]        = ctA[v] ? 1.f / (float)ctA[v] : 0.f;
        const float u  = 1.f + sh;
        ctH[v] = sh < -0.02f ? (uint32_t)((0.015f + 0.25f * u * u) * sr) : 0xFFFFFFFFu;
    }

    // ---- voice parameters
    const float* Pb = prm[BD];
    const float  bdF      = 36.f * pn::Pow(2.6f, Pb[P_TUNE]);           // 36..94 Hz
    const float  bdAc     = Coef(0.07f * pn::Pow(20.f, Pb[P_DECAY]) * dl);
    const float  clickLvl = (punch ? 0.9f : 0.35f) * 1.25f * Pb[P_A];
    const float  bdDrv    = 1.f + 4.f * Pb[P_B] * Pb[P_B];
    const float  bdDrvN   = 1.f / pn::Tanh(bdDrv);
    const float* Ps = prm[SD];
    const float  sdF   = 160.f * pn::Pow(2.1f, Ps[P_TUNE]);             // 160..336 Hz
    const float  sdBc  = Coef((0.05f + 0.18f * Ps[P_DECAY]) * dl);
    const float  snap  = 0.3f + 0.65f * Ps[P_A];
    const float  sdNc  = Coef(0.07f * pn::Pow(7.f, Ps[P_B]) * dl);      // noise tail
    const float  sdHpC = pn::OnePole(sdHpHz, sr);
    const float* Pc = prm[CH];
    const float* Po = prm[OH];
    const float  chScale = 0.7f + 0.9f * Pc[P_TUNE], ohScale = 0.7f + 0.9f * Po[P_TUNE];
    const float  chAc  = Coef(0.012f * pn::Pow(12.f, Pc[P_DECAY]) * dl); // 12..144 ms
    const float  ohAc  = Coef(0.12f * pn::Pow(12.f, Po[P_DECAY]) * dl);  // 0.12..1.4 s
    const float  chHpC = pn::OnePole(5000.f + 4000.f * Pc[P_A], sr);
    const float  ohHpC = pn::OnePole(5000.f + 4000.f * Po[P_A], sr);
    const float  chNz  = pn::Clamp(hatNoise * 2.f * Pc[P_B], 0.f, 1.f);
    const float  ohNz  = pn::Clamp(hatNoise * 2.f * Po[P_B], 0.f, 1.f);
    const float  chokeC = Coef(0.004f);
    const float* Pp = prm[CP];
    const float  cpTone = 800.f * pn::Pow(3.f, Pp[P_TUNE]);
    cp.bp.Set(cpTone, 1.2f + 3.f * Pp[P_B]);
    const uint32_t cpGap = (uint32_t)((0.008f + 0.008f * Pp[P_A]) * sr);
    const float  cpBc  = Coef(0.0035f);
    const float  cpTc  = Coef(0.06f * pn::Pow(7.f, Pp[P_DECAY]) * dl);
    // multi: TUNE sweeps type and pitch
    const float* Pm = prm[MT];
    const float  m  = Pm[P_TUNE];
    int          mType;
    float        mPos;
    if(m < 0.45f)
        mType = 0, mPos = m / 0.45f;
    else if(m < 0.7f)
        mType = 1, mPos = (m - 0.45f) / 0.25f;
    else if(m < 0.85f)
        mType = 2, mPos = (m - 0.7f) / 0.15f;
    else
        mType = 3, mPos = (m - 0.85f) / 0.15f;
    mt.type          = mType;
    const float mDec = 0.05f * pn::Pow(14.f, Pm[P_DECAY]) * dl;
    const float mBend = 2.f * Pm[P_A], mNoise = 2.f * Pm[P_B];
    float       mF;
    switch(mType)
    {
        case 0: mF = 70.f * pn::Pow(3.2f, mPos); break;    // tom
        case 1: mF = 180.f * pn::Pow(2.3f, mPos); break;   // conga
        case 2: mF = 1500.f * pn::Pow(1.8f, mPos); break;  // rim
        default: mF = 540.f * (0.8f + 0.5f * mPos); break; // cowbell
    }
    if(mType == 2)
        mt.bp.Set(mF, 14.f);
    else if(mType == 3)
        mt.bp.Set(mF * 4.8f, 2.f);
    const float mAc   = Coef(mType == 1 ? mDec * 0.45f : (mType == 2 ? mDec * 0.25f : mDec));
    const float mFastC = Coef(0.015f);
    const float mPc   = Coef(mType == 0 ? 0.03f : 0.008f);
    // synths: TUNE 1 V/oct-ish over C2..C7
    const float* Pa = prm[SA];
    const float  synF  = 65.41f * pn::Exp2(Pa[P_TUNE] * 5.f);
    // SA stab: amp decay 80 ms..2 s, filter envelope half as long;
    // CHARACTER B = filter: base cutoff, envelope depth, resonance together
    const float  saDec = 0.08f * pn::Pow(25.f, Pa[P_DECAY]) * dl;
    const float  saAc  = Coef(saDec);
    const float  saFc  = Coef(saDec * 0.45f);
    const float  fB    = Pa[P_B];
    const float  saCut = 140.f * (1.f + 3.f * fB)
                        * pn::Exp2((2.f + 4.5f * fB) * sa.fenv * sa.vel);
    sa.lp.Set(pn::Min(saCut, 16000.f), 0.7f + 6.f * fB * fB);
    // n + 1 saws, normalised to the level of the original 2 n saws
    const float saNorm = 1.f / sqrtf((float)((sa.n + 1) * 2 * sa.n));
    // SB resonator: ring 0.2..6 s, CHARACTER B = brightness (comb damping
    // and the colour of the burst)
    const float* Pz = prm[SB];
    const float  sbF    = 65.41f * pn::Exp2(Pz[P_TUNE] * 5.f);
    const float  sbRing = 0.2f * pn::Pow(30.f, Pz[P_DECAY]) * dl;
    const float  sbDmp  = 0.15f + 0.8f * Pz[P_B];
    float        sbFb[kChordMax];
    for(int j = 0; j < kChordMax; j++)
        sbFb[j] = pn::Pow(0.001f, sb.len[j] / (sbRing * sr));
    const float sbNorm = 1.f / (float)sb.n;
    // echo: dotted 8th of the beat, feedback grows with DUB. Clouds holds
    // 0.5 s: below 88 BPM it falls back to an 8th, then a dotted 16th
    float eDw = bt::st.period * 0.75f;
    if(eDw > (float)(kEch - 64))
        eDw = bt::st.period * 0.5f;
    if(eDw > (float)(kEch - 64))
        eDw = bt::st.period * 0.375f;
    const float eD  = pn::Clamp(eDw, 320.f, (float)(kEch - 64));
    const float eFb = pn::Min(0.75f * echoFb + 0.26f * dub + 0.3f * thr, 0.9f);
    const float eLc = pn::OnePole(3200.f, sr), eHc = pn::OnePole(120.f, sr);

    // ---- triggers: external first (they also mute the pattern), then the
    // pattern on 16th steps
    for(int v = 0; v < kV; v++)
    {
        const float r = req[v];
        if(r > 0.f)
        {
            req[v] = 0.f;
            if(reqExt[v])
                extHold[v] = (uint32_t)(4.f * sr);
            float f = 0.f;
            if(v == SA || v == SB)
            {
                const int   mi = v == SA ? 0 : 1;
                const float mn = midiNote[mi];
                f = mn >= 0.f ? 440.f * pn::Exp2((mn - 69.f) / 12.f)
                              : (v == SA ? synF : sbF);
                midiNote[mi] = -1.f;
            }
            Hit(v, r, f);
        }
        if(extHold[v] > size)
            extHold[v] -= (uint32_t)size;
        else
            extHold[v] = 0;
    }
    {
        const float pos16 = bt::Pos(0, 4) * 4.f;
        int         step  = (int)pos16;
        if(step > 15)
            step = 15;
        if(step != lastStep)
        {
            lastStep = step;
            if(run)
                for(int v = 0; v < kV; v++)
                {
                    if(extHold[v] > 0)
                        continue;
                    const char c = kPat[pat][v][step];
                    if(c < '1' || c > '9')
                        continue;
                    const float vel = (float)(c - '0') / 9.f;
                    float       f   = 0.f;
                    if(v == SA)
                        f = synF * pn::Exp2((float)kNoteA[pat][step] / 12.f);
                    if(v == SB)
                        f = sbF * pn::Exp2((float)kNoteB[pat][step] / 12.f);
                    Hit(v, vel, f);
                }
        }
    }

    // ---- audio in
    const float inC = pn::OnePole(30.f, sr);
    const float c150 = pn::OnePole(150.f, sr), c6k = pn::OnePole(6000.f, sr);
    inMid.Set(sdF * 4.f, 1.2f);
    inCp.Set(cpTone, 2.f);

    // ---- grit (LO-FI kit)
    const float q    = 2048.f; // 12 bit
    const float hInc = 24000.f / sr;
    const float gC   = pn::OnePole(9000.f, sr);
    // unity for small signals (peaks rounded), plus make-up so the three
    // kits play about as loud
    const float dN = (grit ? 1.75f : 1.35f) / busDrive;

    for(size_t i = 0; i < size; i++)
    {
        const float xin = 0.5f * (in[0][i] + in[1][i]);
        const float ax  = fabsf(xin);
        inEnv += (ax - inEnv) * inC;
        // noise source: white, or the input (level-normalised) in SKIN
        float nz = rng.B();
        if(aud == 1)
        {
            const float bl = pn::Clamp(inEnv * 30.f, 0.f, 1.f);
            const float xn = pn::Clamp(xin / (inEnv * 2.5f + 1e-3f), -2.f, 2.f);
            nz += (xn - nz) * bl;
        }

        // BD: pitch-dropping sine + click
        float oL = 0.f, oR = 0.f, rv = 0.f, ev = 0.f, oBd = 0.f;
        auto  mix = [&](int v, float x) {
            {
                uint32_t& n = ctN[v];
                if(n < ctA[v])
                    x *= (float)n * ctAi[v];
                if(n > ctH[v])
                    ctF[v] *= ctFc;
                x *= ctF[v] * lvl[v];
                if(n < 0x7FFFFFFFu)
                    n++;
            }
            if(dComp[v] > 0.f)
                x = Drive(x, dPre[v], dM[v], dComp[v]);
            if(fOn[v])
            {
                float lo, hi;
                vf[v].Run(x, lo, hi);
                x = (fHp[v] ? hi : lo) * fGain[v];
            }
            oL += x * gL[v];
            oR += x * gR[v];
            rv += x * snd[v];
            ev += x * esnd[v];
        };
        if(bd.amp > ampFloor || bd.cl > 1e-4f)
        {
            const float f = bdF * (1.f + bdBend * bd.pe);
            bd.pe *= bdPc;
            bd.ph = Wrap(bd.ph + f * isr);
            float y = pn::Sin2pi(bd.ph) * bd.amp;
            bd.amp *= bdAc;
            y += rng.B() * bd.cl * clickLvl * 0.5f;
            bd.cl *= 0.97f; // ~0.7 ms click
            oBd = pn::Tanh(y * bdDrv) * bdDrvN;
            mix(BD, oBd * kGainBd);
        }
        // SD: two body modes + high-passed noise
        float oSd = 0.f;
        if(sd.body > ampFloor || sd.nz > ampFloor)
        {
            const float f = sdF * (1.f + 0.5f * sd.pe);
            sd.pe *= 0.995f;
            sd.p1 = Wrap(sd.p1 + f * isr);
            sd.p2 = Wrap(sd.p2 + f * 1.62f * isr);
            const float body = (0.6f * pn::Sin2pi(sd.p1) + 0.35f * pn::Sin2pi(sd.p2))
                               * sd.body;
            sd.body *= sdBc;
            sd.lp += (nz - sd.lp) * sdHpC;
            const float n = (nz - sd.lp) * sd.nz;
            sd.nz *= sdNc;
            oSd = body * (1.f - 0.45f * snap) + n * snap * 1.1f;
            mix(SD, oSd * kGainSd);
        }
        // hats: six detuned squares (808 ratios) + noise, high-passed twice
        static const float kMetal[6] = {205.3f, 304.4f, 369.6f, 522.7f, 540.f, 800.f};
        Hat* hats[2]     = {&ch, &oh};
        const float sc[2] = {chScale, ohScale}, hc[2] = {chHpC, ohHpC};
        const float ac[2] = {chAc, ohAc}, hn[2] = {chNz, ohNz};
        for(int h = 0; h < 2; h++)
        {
            Hat& hh = *hats[h];
            if(hh.amp < ampFloor)
                continue;
            float sum = 0.f;
            for(int j = 0; j < 6; j++)
            {
                hh.ph[j] = Wrap(hh.ph[j] + kMetal[j] * sc[h] * isr);
                sum += Sq(hh.ph[j]);
            }
            float x = sum * (1.f / 6.f) * (1.f - hn[h]) + nz * hn[h];
            hh.h1 += (x - hh.h1) * hc[h];
            const float y1 = x - hh.h1;
            hh.h2 += (y1 - hh.h2) * hc[h];
            const float y2 = y1 - hh.h2;
            hh.amp *= ac[h];
            if(hh.choke < 1.f)
                hh.amp *= chokeC;
            const float yh = y2 * hh.amp * hatG;
            mix(h == 0 ? CH : OH, yh);
        }
        // CP: three quick bursts, then a tail, band-passed noise
        float oCp = 0.f;
        if(cp.t < (uint32_t)(3.f * sr))
        {
            if(cp.t == cpGap || cp.t == 2 * cpGap)
                cp.b = pn::Max(cp.b, 0.85f * cp.vel);
            if(cp.t == 3 * cpGap)
                cp.tail = 0.75f * cp.vel;
            const float env = pn::Max(cp.b, cp.tail);
            cp.b *= cpBc;
            cp.tail *= cpTc;
            float lo, hi;
            oCp = cp.bp.Run(nz, lo, hi) * env * 2.2f;
            cp.t++;
            if(env < ampFloor && cp.t > 3 * cpGap)
                cp.t = 99999;
            oCp *= kGainCp;
            mix(CP, oCp);
        }
        // MT: tom / conga / rim / cowbell
        float oMt = 0.f;
        if(mt.amp > ampFloor)
        {
            float y;
            switch(mt.type)
            {
                case 0: // tom: sine with a deep pitch drop
                case 1: // conga: higher, tighter, a slap of noise
                {
                    const float bend = (mt.type == 0 ? 0.6f : 0.15f) * mBend;
                    mt.ph = Wrap(mt.ph + mF * (1.f + bend * mt.pe) * isr);
                    y     = pn::Sin2pi(mt.ph);
                    if(mt.type == 1)
                    {
                        y += nz * mt.slap * 0.4f * mNoise;
                        mt.slap *= 0.993f;
                    }
                    else
                        y += nz * 0.04f * mNoise;
                    mt.pe *= mPc;
                    break;
                }
                case 2: // rim: ringing band-pass ping
                {
                    float lo, hi;
                    const float ex = mt.ping ? 1.f : 0.f;
                    mt.ping        = false;
                    y              = mt.bp.Run(ex * 8.f + nz * mt.fast * 0.3f * mNoise, lo, hi);
                    break;
                }
                default: // cowbell: two squares through a band-pass
                {
                    mt.ph  = Wrap(mt.ph + mF * isr);
                    mt.ph2 = Wrap(mt.ph2 + mF * 1.48f * isr);
                    float lo, hi;
                    y = mt.bp.Run(0.5f * (Sq(mt.ph) + Sq(mt.ph2)), lo, hi) * 1.6f;
                    y *= 0.35f + 0.65f * mt.fast;
                    break;
                }
            }
            mt.fast *= mFastC;
            oMt = y * mt.amp;
            mt.amp *= mAc;
            oMt *= kGainMt;
            mix(MT, oMt);
        }
        // SA: chord stab, detuned saws -> resonant low-pass
        if(sa.amp > ampFloor)
        {
            float sum = 0.f;
            for(int j = 0; j < sa.n; j++)
                for(int u = 0; u < (j == 0 ? 2 : 1); u++)
                {
                    float&      ph = sa.ph[j][u];
                    const float dt = sa.inc[j][u];
                    ph             = Wrap(ph + dt);
                    sum += 2.f * ph - 1.f - Blep(ph, dt);
                }
            float lo, hi;
            sa.lp.Run(sum * saNorm, lo, hi);
            const float y = pn::Tanh(lo * 1.4f) * sa.amp;
            sa.amp *= saAc;
            sa.fenv *= saFc;
            mix(SA, y * kGainSa);
        }
        // SB: resonator chord (one comb per chord note)
        if(sb.act > 0)
        {
            float x = 0.f;
            if(sb.exc > 0)
            {
                sb.nlp += (rng.B() - sb.nlp) * sbDmp;
                x = sb.nlp * sb.amp;
                sb.exc--;
            }
            float sum = 0.f;
            for(int j = 0; j < sb.n; j++)
            {
                const float y = pn::ReadFrac(rcBuf[j], kRcM, sb.w, sb.len[j]);
                sb.lp[j] += (y - sb.lp[j]) * sbDmp;
                rcBuf[j][sb.w & kRcM] = pn::LoopClip(x + sb.lp[j] * sbFb[j]);
                sum += y;
            }
            sb.w++;
            sb.act--;
            sbPeak = pn::Max(sbPeak, fabsf(sum));
            mix(SB, sum * sbNorm * kGainSb);
        }

        // ---- audio in: THRU / SKIN (ducked by the kick) / CHOP
        float dry = 0.f;
        if(aud == 0)
            dry = xin * 0.5f;
        else if(aud == 1)
            dry = xin * 0.5f * (1.f - 0.7f * pn::Min(bd.amp, 1.f));
        else
        {
            lo1 += (xin - lo1) * c150;
            lo2 += (lo1 - lo2) * c150;
            hiLp += (xin - hiLp) * c6k;
            float l, h;
            const float mid  = inMid.Run(xin, l, h);
            const float cpb  = inCp.Run(xin, l, h);
            const float hatE = ch.amp + oh.amp;
            dry = lo2 * 1.6f * pn::Min(bd.amp, 1.f) + mid * 1.4f * pn::Max(sd.body, sd.nz)
                  + (xin - hiLp) * 1.6f * pn::Min(hatE, 1.f)
                  + cpb * 1.4f * pn::Max(cp.b, cp.tail) + xin * 0.8f * pn::Max(mt.amp, sa.amp);
            dry *= 0.6f; // headroom: the chopped input sits under the drums
        }

        // ---- bus: drive, LO-FI grit, then the (clean) reverb
        float yo[2] = {oL, oR};
        if(busDrive > 1.01f)
            for(int c = 0; c < 2; c++)
                yo[c] = pn::Tanh(yo[c] * busDrive) * dN;
        if(grit)
        {
            holdPh += hInc;
            if(holdPh >= 1.f)
            {
                holdPh -= 1.f;
                held[0] = pn::Floor(yo[0] * q + 0.5f) * (1.f / q);
                held[1] = pn::Floor(yo[1] * q + 0.5f) * (1.f / q);
            }
            for(int c = 0; c < 2; c++)
            {
                gLp[c] += (held[c] - gLp[c]) * gC;
                yo[c] = gLp[c];
            }
        }
        // SEND bus: ping-pong echo, then echo + send into the reverb
        eDs += (eD - eDs) * 0.0005f; // glide on tempo changes
        const float eL = 2.f * pn::ReadFracS(ech[0], kEchM, ew, eDs); // stored at half level
        const float eR = 2.f * pn::ReadFracS(ech[1], kEchM, ew, eDs);
        float       fbk[2] = {eR * eFb, eL * eFb}; // cross = ping-pong
        for(int c = 0; c < 2; c++)
        {
            eLp[c] += (fbk[c] - eLp[c]) * eLc;
            eHp[c] += (eLp[c] - eHp[c]) * eHc;
            fbk[c] = eLp[c] - eHp[c];
        }
        ech[0][ew & kEchM] = pn::F2S(0.5f * pn::LoopClip(ev + fbk[0]));
        ech[1][ew & kEchM] = pn::F2S(0.5f * pn::LoopClip(fbk[1]));
        ew++;
        float rl, rr;
        Reverb(rv + 0.5f * (eL + eR), rvFb, rl, rr);
        rl += 0.4f * eL;
        rr += 0.4f * eR;
        // DUB pulls the dry drums down a little (like a desk throw), so the
        // reverb can come up without hitting the limiter
        const float dd = (1.f - 0.3f * dub) * (1.f - mute);
        out[0][i] = pn::Lim(yo[0] * dd + rl * 0.45f + dry);
        out[1][i] = pn::Lim(yo[1] * dd + rr * 0.45f + dry);
    }
    // the resonator has rung out (and no burst is left): stop computing it
    // (not before 50 ms: the combs are up to 32 ms long, so they start
    // ringing well after the burst)
    if(sb.act > 0 && sb.exc <= 0 && sbPeak < ampFloor
       && sb.act + (uint32_t)(0.05f * sr) < (uint32_t)(8.f * sr))
        sb.act = 0;
    sbPeak = 0.f;
}

inline void Leds(LedOut& l, uint32_t now, float bf)
{
    // 0 kick / snare, 1 hats, 2 clap / multi, 3 synths; dim glow = pattern
    // running (colour per pattern), dark = stopped
    auto d = [](int v) { float x = hitShow[v]; hitShow[v] = x * 0.8f; return x; };
    const float bdv = d(BD), sdv = d(SD), chv = d(CH), ohv = d(OH);
    const float cpv = d(CP), mtv = d(MT), sav = d(SA), sbv = d(SB);
    const float g   = showRun ? 0.06f : 0.f;
    l.Set(0, pn::Min(1.f, bdv + g), pn::Min(1.f, sdv * 0.6f + g), g);
    l.Set(1, pn::Min(1.f, chv * 0.8f + ohv), pn::Min(1.f, chv * 0.8f + ohv), pn::Min(1.f, chv * 0.3f + g));
    l.Set(2, pn::Min(1.f, cpv), pn::Min(1.f, mtv + g), pn::Min(1.f, cpv * 0.5f + mtv * 0.6f));
    // LED 3 = the voice the knobs edit (page), hit of that voice = flash
    static const float kPg[kV][3] = {
        {1.f, 0.f, 0.f},    // BD red
        {1.f, 0.4f, 0.f},   // SD orange
        {1.f, 1.f, 0.6f},   // CH pale yellow
        {1.f, 0.8f, 0.f},   // OH yellow
        {0.f, 1.f, 1.f},    // CP cyan
        {0.f, 1.f, 0.f},    // MT green
        {0.7f, 0.f, 1.f},   // SA violet
        {0.f, 0.3f, 1.f},   // SB blue
    };
    const int   pg = page;
    const float hv[kV] = {bdv, sdv, chv, ohv, cpv, mtv, sav, sbv};
    const float b3 = 0.35f + 0.65f * pn::Min(hv[pg] * 2.f, 1.f);
    l.Set(3, kPg[pg][0] * b3, kPg[pg][1] * b3, kPg[pg][2] * b3);
    // DUB: blue wash over LEDs 0..2
    const float dw = dub * 0.6f;
    for(int k = 0; k < 3; k++)
        l.Set(k, l.c[k][0], l.c[k][1], pn::Min(1.f, l.c[k][2] + dw));
    (void)now;
    (void)bf;
}

} // namespace tdr
