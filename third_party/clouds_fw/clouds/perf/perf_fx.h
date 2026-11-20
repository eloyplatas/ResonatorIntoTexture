// perf_fx.h - momentary performance effects on the final stereo output of
// TRIG DRUMS and DATA BENDER. Hold FREEZE, BLEND-select or LOAD/SAVE and
// turn a knob: how far the knob moved from where it was when the button
// went down sets the amount (and, for some, the direction). Releasing the
// button lets everything snap back.
//
//            FREEZE           BLEND-select       LOAD/SAVE
//  POSITION  STUTTER 1/8-1/32  GATE (synced)      SKIP (random 16ths)
//  SIZE      TAPE STOP        REVERSE            FLANGER
//  PITCH     PITCH BEND +-1o  VIBRATO / WOW      OCTAVE up / down
//  DENSITY   FILTER LP <> HP  RING MOD           BANDPASS sweep
//  TEXTURE   BITCRUSH         DRIVE / FOLD       SHUFFLE slices
//  BLEND     WASH (echoes)    AUTOPAN / MONO     PUMP (sidechain)
//
// The knob sets the effect from its absolute position as soon as it moves
// (jump), and every effect starts from the same initial state each time it
// is engaged (fresh capture, full speed, LFOs at phase 0). Knobs with a
// direction (pitch bend, filter, vibrato / wow, autopan / mono, octave) are
// neutral at noon.
//
// One int16 stereo buffer of 8192 frames (0.26 s) from the arena feeds every
// effect that plays back the past. Replacing effects (stutter, reverse, tape
// stop, shuffle, pitch bend, vibrato) take turns in that priority; the
// others stack.
#pragma once
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "clouds/bender/pn.h"
#include "clouds/bender/arena.h"

namespace pfx
{
enum Button
{
    B_FREEZE,
    B_MODE,
    B_WRITE,
    kButtons
};
enum Knob
{
    K_POSITION,
    K_SIZE,
    K_PITCH,
    K_DENSITY,
    K_TEXTURE,
    K_BLEND,
    kKnobs
};

// written once per block by the firmware glue
struct Ctl
{
    float amt[kButtons][kKnobs]; // 0..1
    float sgn[kButtons][kKnobs]; // -1 / +1: direction the knob went
    float beat;                  // samples per beat
    float phase;                 // beat phase 0..1 at the block start
    bool  cycle;                 // toggle presets: the capture effects
                                 // renew every beat (never sound stuck)
};

static constexpr uint32_t kN = 8192, kM = kN - 1; // 0.256 s at 32 kHz
static int16_t*           rb[2] = {nullptr, nullptr};
static uint32_t           w     = 0;
static float              sr    = 32000.f;
static pn::Rng            rng;

// smoothed amounts and engage flags
static float sm[kButtons][kKnobs];
static bool  on[kButtons][kKnobs];

// effect state
static uint32_t stEnd = 0, stLen = 4000, stT = 0;       // stutter: loops the
                                                        // stLen samples before stEnd
static uint32_t rvStart = 0, rvLen = 8000, rvT = 0;     // reverse
static float    tpPos = 0.f, tpSpeed = 1.f;             // tape stop
static bool     stCap = false, rvCap = false;           // cycle: capturing
static uint32_t beatN = 0;                              // cycle: beats
static uint32_t shOff = 0, shT = 0, shLen = 4000;       // shuffle
static float    psP = 0.f, ocP = 0.f;                   // pitch shifters
static float    vbPh = 0.f, flPh = 0.f, rmPh = 0.f;
static float    xf = 1.f;                               // replace fade
static float    rHold[2] = {0, 0};                      // last replaced output
static int      lastRep = -1;
static float    holdPh = 0.f, held[2] = {0, 0};         // bitcrush
static float    gateG = 1.f, skipG = 1.f;
static bool     skipMute = false;
static int      lastSixteenth = -1;
static float    washLp[2] = {0, 0}, washFb[2] = {0, 0};
static float    flFb[2] = {0, 0};

struct Svf
{
    float ic1 = 0.f, ic2 = 0.f, a1 = 0.9f, ga1 = 0.09f, k = 1.f;
    void  Set(float hz, float q)
    {
        const float g = pn::Tan(3.1415927f * pn::Clamp(hz, 10.f, sr * 0.45f) / sr);
        k             = 1.f / q;
        a1            = 1.f / (1.f + g * (g + k));
        ga1           = g * a1;
    }
    float Run(float x, float& lo, float& hi)
    {
        const float v3 = x - ic2;
        const float v1 = a1 * ic1 + ga1 * v3;
        const float v2 = ic2 + (ga1 / a1) * v1;
        ic1            = 2.f * v1 - ic1;
        ic2            = 2.f * v2 - ic2;
        lo             = v2;
        hi             = x - k * v1 - v2;
        return v1;
    }
    void Reset() { ic1 = ic2 = 0.f; }
};
static Svf fl[2], bp[2], wl[2];

inline float Rd(int c, uint32_t back) // back >= 1
{
    return (float)rb[c][(w - back) & kM] * (1.f / 32767.f);
}
inline float RdF(int c, float back) // fractional, back >= 1
{
    const uint32_t i  = (uint32_t)back;
    const float    f  = back - (float)i;
    const float    a  = (float)rb[c][(w - i) & kM];
    const float    b2 = (float)rb[c][(w - i - 1) & kM];
    return (a + (b2 - a) * f) * (1.f / 32767.f);
}
inline float RdAbs(int c, uint32_t idx)
{
    return (float)rb[c][idx & kM] * (1.f / 32767.f);
}
// 2 ms fade at both ends of a loop of len samples, t inside it
inline float Win(uint32_t t, uint32_t len)
{
    const uint32_t f = 64;
    if(t < f)
        return (float)t * (1.f / 64.f);
    if(len - t < f)
        return (float)(len - t) * (1.f / 64.f);
    return 1.f;
}

inline void Reset()
{
    memset(sm, 0, sizeof(sm));
    memset(on, 0, sizeof(on));
    w = 0, xf = 1.f, lastRep = -1;
    tpSpeed = 1.f;
    gateG = skipG = 1.f;
    for(int c = 0; c < 2; c++)
    {
        fl[c].Reset(), bp[c].Reset(), wl[c].Reset();
        washLp[c] = washFb[c] = flFb[c] = 0.f;
        held[c] = 0.f;
    }
}
// call when the mode starts (buffers zeroed by the arena)
inline void Attach()
{
    rb[0] = arena::M.Take<int16_t>(kN);
    rb[1] = arena::M.Take<int16_t>(kN);
    Reset();
}
inline bool Busy()
{
    for(int b = 0; b < kButtons; b++)
        for(int k = 0; k < kKnobs; k++)
            if(sm[b][k] > 0.01f)
                return true;
    return false;
}

inline uint32_t ClampLen(float len)
{
    return (uint32_t)pn::Clamp(len, 256.f, (float)(kN - 300));
}

// an effect is engaged: put it in its initial state
inline void Engage(int b, int k, float beat)
{
    switch(b * kKnobs + k)
    {
        case B_FREEZE * kKnobs + K_POSITION: // stutter: capture now
            stT = 0, stLen = ClampLen(beat * 0.5f), stEnd = w, stCap = false;
            break;
        case B_FREEZE * kKnobs + K_SIZE: // tape stop: full speed
            tpPos = (float)w, tpSpeed = 1.f, beatN = 0;
            break;
        case B_FREEZE * kKnobs + K_PITCH: psP = 0.f; break;
        case B_FREEZE * kKnobs + K_DENSITY: fl[0].Reset(), fl[1].Reset(); break;
        case B_FREEZE * kKnobs + K_TEXTURE: holdPh = 1.f; break;
        case B_FREEZE * kKnobs + K_BLEND:
            washFb[0] = washFb[1] = 0.f, wl[0].Reset(), wl[1].Reset();
            break;
        case B_MODE * kKnobs + K_POSITION: gateG = 1.f; break;
        case B_MODE * kKnobs + K_SIZE: // reverse: capture now
            rvT = 0, rvLen = ClampLen(beat * 0.5f), rvStart = w - rvLen, rvCap = false;
            break;
        case B_MODE * kKnobs + K_PITCH: vbPh = 0.f; break;
        case B_MODE * kKnobs + K_DENSITY: rmPh = 0.f; break;
        case B_WRITE * kKnobs + K_POSITION: lastSixteenth = -1; break;
        case B_WRITE * kKnobs + K_SIZE: flPh = 0.f, flFb[0] = flFb[1] = 0.f; break;
        case B_WRITE * kKnobs + K_PITCH: ocP = 0.f; break;
        case B_WRITE * kKnobs + K_DENSITY: bp[0].Reset(), bp[1].Reset(); break;
        case B_WRITE * kKnobs + K_TEXTURE: shT = 0; break;
        default: break;
    }
}

// which knobs have a direction (neutral at noon)
static const bool kBipolar[kButtons][kKnobs] = {
    {false, false, true, true, false, false},  // FREEZE: pitch bend, filter
    {false, false, true, false, false, true},  // BLEND-sel.: vibrato, autopan
    {false, false, true, false, false, false}, // LOAD/SAVE: octave
};

// knobs -> FX amounts for the buttons held now: a knob takes over once it
// has moved 0.02 since the button went down, then its absolute position is
// the amount (and the direction, around noon, for the bipolar ones).
// eng[][] remembers which knobs took over (cleared when the button is up).
inline void KnobsToFx(const bool held[kButtons], const float* kv,
                      const float* press, bool eng[kButtons][kKnobs],
                      volatile float amt[kButtons][kKnobs],
                      volatile float sgn[kButtons][kKnobs],
                      volatile bool used[kButtons])
{
    for(int b = 0; b < kButtons; b++)
        for(int k = 0; k < kKnobs; k++)
        {
            if(!held[b])
                eng[b][k] = false;
            else if(!eng[b][k] && fabsf(kv[k] - press[k]) > 0.02f)
                eng[b][k] = true, used[b] = true;
            float a = 0.f;
            if(eng[b][k])
            {
                const float x = pn::Clamp(kv[k], 0.f, 1.f);
                if(kBipolar[b][k])
                {
                    const float d = 2.f * x - 1.f;
                    a             = pn::Clamp((fabsf(d) - 0.04f) * (1.f / 0.96f), 0.f, 1.f);
                    sgn[b][k]     = d < 0.f ? -1.f : 1.f;
                }
                else
                {
                    a         = x;
                    sgn[b][k] = 1.f;
                }
            }
            amt[b][k] = a;
        }
}

inline void Process(const Ctl& c, float* L, float* R, size_t n)
{
    // idle (nothing held, nothing fading): only keep recording the buffer
    bool idle = xf >= 1.f && washFb[0] == 0.f && flFb[0] == 0.f
                && gateG > 0.999f && skipG > 0.999f;
    for(int b = 0; b < kButtons && idle; b++)
        for(int k = 0; k < kKnobs; k++)
            if(c.amt[b][k] > 0.f || sm[b][k] > 0.f)
            {
                idle = false;
                break;
            }
    if(idle)
    {
        for(size_t i = 0; i < n; i++)
        {
            rb[0][w & kM] = pn::F2S(0.5f * L[i]);
            rb[1][w & kM] = pn::F2S(0.5f * R[i]);
            w++;
        }
        lastRep = -1;
        return;
    }
    const float beat = pn::Clamp(c.beat, 8000.f, 64000.f); // 30..240 BPM
    // ---- amounts: ~4 ms smoothing, engage / release events
    for(int b = 0; b < kButtons; b++)
        for(int k = 0; k < kKnobs; k++)
        {
            const float t = c.amt[b][k];
            sm[b][k] += (t - sm[b][k]) * 0.25f;
            if(t < 0.01f && sm[b][k] < 0.005f)
                sm[b][k] = 0.f;
            // hysteresis: a knob resting near its zero point must not
            // switch the effect on and off (each engage restarts it)
            const bool o = on[b][k] ? t > 0.01f : t > 0.04f;
            if(o && !on[b][k])
                Engage(b, k, beat); // always the same start
            on[b][k] = o;
        }
    const float* A  = sm[B_FREEZE];
    const float* Bm = sm[B_MODE];
    const float* Lw = sm[B_WRITE];

    // replacing effect in charge (priority order)
    int rep = -1;
    if(on[B_FREEZE][K_POSITION])
        rep = 0; // stutter
    else if(on[B_MODE][K_SIZE])
        rep = 1; // reverse
    else if(on[B_FREEZE][K_SIZE])
        rep = 2; // tape stop
    else if(on[B_WRITE][K_TEXTURE])
        rep = 3; // shuffle
    else if(on[B_FREEZE][K_PITCH])
        rep = 4; // pitch bend
    else if(on[B_MODE][K_PITCH])
        rep = 5; // vibrato
    if(rep != lastRep)
        xf = 0.f; // fade the new source in (2 ms)
    lastRep = rep;

    // stutter roll: 1/8, 1/16, 1/32 notes as the knob goes further; a new
    // length starts at the next loop start (no jump inside the loop)
    uint32_t stNext = stLen;
    if(rep == 0)
    {
        const float a   = c.amt[B_FREEZE][K_POSITION];
        const float div = a < 0.4f ? 0.5f : (a < 0.75f ? 0.25f : 0.125f);
        stNext          = ClampLen(beat * div);
    }
    // tape stop: stops in 0.6 s (a little turn) .. 0.1 s (a big turn)
    const float tpRate = 1.f / ((0.6f - 0.5f * c.amt[B_FREEZE][K_SIZE]) * sr);
    // pitch shifters
    const float psRatio = pn::Exp2(A[K_PITCH] * c.sgn[B_FREEZE][K_PITCH]);
    const float ocRatio = c.sgn[B_WRITE][K_PITCH] > 0.f ? 2.f : 0.5f;
    const float W       = 1024.f;
    // vibrato / wow
    const bool  wow     = c.sgn[B_MODE][K_PITCH] < 0.f;
    const float vbInc   = (wow ? 0.8f : 6.f) / sr;
    const float vbDepth = Bm[K_PITCH] * (wow ? 96.f : 48.f); // samples
    // flanger: 0.25 Hz sweep, 1..5 ms
    const float flInc = 0.25f / sr;
    // filter sweep
    const float fa = A[K_DENSITY];
    const bool  hp = c.sgn[B_FREEZE][K_DENSITY] > 0.f;
    if(fa > 0.005f)
        for(int ch = 0; ch < 2; ch++)
            fl[ch].Set(hp ? 20.f * pn::Exp2(fa * 8.f) : 18000.f * pn::Exp2(-fa * 7.5f),
                       0.8f + 2.5f * fa);
    // bandpass sweep 200 Hz .. 4 kHz
    const float ba = Lw[K_DENSITY];
    if(ba > 0.005f)
        for(int ch = 0; ch < 2; ch++)
            bp[ch].Set(200.f * pn::Exp2(ba * 4.3f), 1.6f);
    // bitcrush: bits 16 -> 6, rate 32 kHz -> 4 kHz
    const float ca    = A[K_TEXTURE];
    const float cRate = 1.f - ca * 0.875f;
    const float cQ    = pn::Exp2(15.f - ca * 9.f);
    // drive / fold
    const float da = Bm[K_TEXTURE];
    const float dPre = 1.f + 14.f * da * da;
    // level match, fitted on a drum loop: louder in the middle (saturation),
    // quieter at the end (folding)
    const float dComp = da < 0.5f ? 1.f / (1.f + 4.6f * da) : 0.3f + 0.75f * (da - 0.5f);
    // ring mod 30 Hz .. 2 kHz
    const float rmInc = 30.f * pn::Exp2(Bm[K_DENSITY] * 6.f) / sr;
    // gate: 1/8, 1/16, 1/32 notes
    const float ga   = Bm[K_POSITION];
    const float gDiv = ga < 0.4f ? 2.f : (ga < 0.75f ? 4.f : 8.f);
    // autopan (1 beat) / mono
    const float pa   = Bm[K_BLEND];
    const bool  mono = c.sgn[B_MODE][K_BLEND] < 0.f;
    // wash: echoes at 67 / 101 / 149 ms, feedback grows with the amount
    const float wa = A[K_BLEND];
    if(wa > 0.005f)
        for(int ch = 0; ch < 2; ch++)
            wl[ch].Set(3500.f, 0.6f);
    // pump: duck at every beat
    const float pu = Lw[K_BLEND];
    const float sk = Lw[K_POSITION];
    const float fla = Lw[K_SIZE];
    const float oca = Lw[K_PITCH];

    const float binc = 1.f / beat;
    float       ph   = c.phase;

    for(size_t i = 0; i < n; i++)
    {
        const float x[2] = {L[i], R[i]};
        ph += binc;
        const bool wrap = ph >= 1.f;
        if(wrap)
            ph -= 1.f;
        if(wrap && c.cycle)
        {
            // toggle presets: every beat the capture effects start over
            // from the music playing now (a beat repeat, not a frozen loop)
            if(rep == 0)
                stCap = true, stT = 0, stLen = stNext, stEnd = w + stLen;
            if(rep == 1)
                rvCap = true, rvT = 0, rvLen = ClampLen(beat * 0.5f), rvStart = w;
            if(rep == 2 && (++beatN & 1) == 0)
                tpPos = (float)w, tpSpeed = 1.f; // every 2 beats: run again
        }
        // ---- write (wash + flanger feedback go back into the buffer)
        // stutter and reverse play captured audio: the recording pauses
        // while they loop, so the capture is never overwritten
        if(!((rep == 0 && !stCap) || (rep == 1 && !rvCap)))
        {
            for(int ch = 0; ch < 2; ch++)
                rb[ch][w & kM] = pn::F2S(0.5f * pn::LoopClip(x[ch] + washFb[ch] + flFb[ch]));
            w++;
        }

        float y[2] = {x[0], x[1]};
        // ---- replacing sources
        if(rep >= 0)
        {
            float r[2];
            switch(rep)
            {
                case 0:
                {
                    if(stCap)
                    {
                        // capturing: the slice plays live once (faded
                        // at its ends like the repeats)
                        const float gc = Win(stT, stLen);
                        r[0] = x[0] * gc, r[1] = x[1] * gc;
                        if(++stT >= stLen)
                            stCap = false, stT = 0;
                        break;
                    }
                    const float g = Win(stT, stLen);
                    for(int ch = 0; ch < 2; ch++)
                        r[ch] = 2.f * RdAbs(ch, stEnd - stLen + stT) * g;
                    if(++stT >= stLen)
                        stT = 0, stLen = stCap ? stLen : (c.cycle ? stLen : stNext);
                    break;
                }
                case 1:
                {
                    if(rvCap)
                    {
                        const float gc = Win(rvT, rvLen);
                        r[0] = x[0] * gc, r[1] = x[1] * gc;
                        if(++rvT >= rvLen)
                            rvCap = false, rvT = 0;
                        break;
                    }
                    const float g = Win(rvT, rvLen);
                    for(int ch = 0; ch < 2; ch++)
                        r[ch] = 2.f * RdAbs(ch, rvStart + rvLen - 1 - rvT) * g;
                    if(++rvT >= rvLen)
                        rvT = 0;
                    break;
                }
                case 2:
                {
                    tpSpeed = pn::Max(0.f, tpSpeed - tpRate);
                    tpPos += tpSpeed;
                    const float back = (float)w - tpPos;
                    const float g    = tpSpeed > 0.f && back < (float)(kN - 8)
                                        ? pn::Min(1.f, tpSpeed * 3.f) : 0.f;
                    for(int ch = 0; ch < 2; ch++)
                        r[ch] = g > 0.f ? 2.f * RdF(ch, pn::Max(1.f, back)) * g : 0.f;
                    break;
                }
                case 3:
                {
                    // every 16th: replay one of the last 16ths (or live)
                    const float a = c.amt[B_WRITE][K_TEXTURE];
                    if(shT == 0)
                    {
                        shLen = ClampLen(beat * 0.25f);
                        const uint32_t maxK = (kN - 300) / shLen;
                        shOff = (maxK > 0 && rng.F() < a)
                                    ? shLen * (1 + rng.Next() % (maxK > 3 ? 3 : maxK))
                                    : 0;
                    }
                    const float g = shOff ? Win(shT, shLen) : 1.f;
                    for(int ch = 0; ch < 2; ch++)
                        r[ch] = shOff ? 2.f * Rd(ch, shOff + 1) * g : x[ch];
                    if(++shT >= shLen)
                        shT = 0;
                    break;
                }
                case 4:
                {
                    psP += 1.f - psRatio;
                    if(psP >= W)
                        psP -= W;
                    if(psP < 0.f)
                        psP += W;
                    float p2 = psP + 0.5f * W;
                    if(p2 >= W)
                        p2 -= W;
                    const float g1 = pn::Tri(psP / W), g2 = 1.f - g1;
                    for(int ch = 0; ch < 2; ch++)
                        r[ch] = 2.f * (RdF(ch, psP + 2.f) * g1 + RdF(ch, p2 + 2.f) * g2);
                    break;
                }
                default:
                {
                    vbPh += vbInc;
                    if(vbPh >= 1.f)
                        vbPh -= 1.f;
                    const float d = 200.f + vbDepth * (1.f + pn::Sin2pi(vbPh));
                    for(int ch = 0; ch < 2; ch++)
                        r[ch] = 2.f * RdF(ch, d);
                    break;
                }
            }
            xf = pn::Min(1.f, xf + 1.f / 64.f);
            for(int ch = 0; ch < 2; ch++)
                y[ch] = x[ch] + (r[ch] - x[ch]) * xf, rHold[ch] = y[ch];
        }
        else if(xf < 1.f)
        {
            // released: fade from the last replaced sample back to the dry
            // signal over 2 ms
            xf = pn::Min(1.f, xf + 1.f / 64.f);
            for(int ch = 0; ch < 2; ch++)
                y[ch] = x[ch] + (rHold[ch] - x[ch]) * (1.f - xf);
        }

        // ---- added: octave, flanger, wash
        if(oca > 0.005f)
        {
            ocP += 1.f - ocRatio;
            if(ocP >= W)
                ocP -= W;
            if(ocP < 0.f)
                ocP += W;
            float p2 = ocP + 0.5f * W;
            if(p2 >= W)
                p2 -= W;
            const float g1 = pn::Tri(ocP / W), g2 = 1.f - g1;
            for(int ch = 0; ch < 2; ch++)
                y[ch] += oca * 2.f * (RdF(ch, ocP + 2.f) * g1 + RdF(ch, p2 + 2.f) * g2);
        }
        if(fla > 0.005f)
        {
            flPh += flInc;
            if(flPh >= 1.f)
                flPh -= 1.f;
            for(int ch = 0; ch < 2; ch++)
            {
                const float ph2 = flPh + (ch ? 0.25f : 0.f);
                const float d   = 32.f + 64.f * (1.f + pn::Sin2pi(ph2));
                const float f   = 2.f * RdF(ch, d);
                y[ch] += fla * f;
                flFb[ch] = 0.6f * fla * f;
            }
        }
        else
            flFb[0] = flFb[1] = 0.f;
        if(wa > 0.005f)
        {
            for(int ch = 0; ch < 2; ch++)
            {
                const float e = 2.f
                                * (0.4f * Rd(ch, ch ? 2144 : 3232) + 0.35f * Rd(ch, 4768)
                                   + 0.25f * Rd(ch, ch ? 3232 : 2144));
                float lo, hi;
                wl[ch].Run(e, lo, hi);
                washLp[ch] = lo;
                y[ch] += wa * 1.2f * lo;
                washFb[ch] = (0.35f + 0.5f * wa) * wa * lo;
            }
        }
        else
            washFb[0] = washFb[1] = 0.f;

        // ---- tone
        if(fa > 0.005f)
            for(int ch = 0; ch < 2; ch++)
            {
                float lo, hi;
                fl[ch].Run(y[ch], lo, hi);
                y[ch] += ((hp ? hi : lo) - y[ch]) * pn::Min(1.f, fa * 6.f);
            }
        if(ba > 0.005f)
            for(int ch = 0; ch < 2; ch++)
            {
                float lo, hi;
                const float b = bp[ch].Run(y[ch], lo, hi);
                y[ch] += (1.6f * b - y[ch]) * pn::Min(1.f, ba * 4.f);
            }
        if(ca > 0.005f)
        {
            holdPh += cRate;
            if(holdPh >= 1.f)
            {
                holdPh -= 1.f;
                for(int ch = 0; ch < 2; ch++)
                    held[ch] = pn::Floor(y[ch] * cQ + 0.5f) / cQ;
            }
            const float m = pn::Min(1.f, ca * 6.f);
            for(int ch = 0; ch < 2; ch++)
                y[ch] += (held[ch] - y[ch]) * m;
        }
        if(da > 0.005f)
            for(int ch = 0; ch < 2; ch++)
            {
                const float z   = pn::Tanh(y[ch] * dPre);
                const float fld = pn::Sin(z * (1.5f + 2.5f * da));
                // level matched: about as loud as the dry signal
                const float wet = (z + (fld - z) * da * 0.6f) * dComp;
                y[ch] += (wet - y[ch]) * pn::Min(1.f, da * 4.f);
            }
        if(Bm[K_DENSITY] > 0.005f)
        {
            rmPh += rmInc;
            if(rmPh >= 1.f)
                rmPh -= 1.f;
            const float m = pn::Sin2pi(rmPh);
            const float a = pn::Min(1.f, Bm[K_DENSITY] * 3.f);
            for(int ch = 0; ch < 2; ch++)
                y[ch] += (y[ch] * m - y[ch]) * a;
        }

        // ---- level: gate, skip, pump (beat synced)
        if(ga > 0.005f)
        {
            float gp = ph * gDiv;
            gp -= pn::Floor(gp);
            const float tg = gp < 0.5f ? 1.f : 1.f - pn::Min(1.f, ga * 3.f);
            gateG += (tg - gateG) * 0.06f;
        }
        else
            gateG += (1.f - gateG) * 0.06f;
        if(sk > 0.005f)
        {
            const int s16 = (int)(ph * 4.f);
            if(s16 != lastSixteenth)
            {
                lastSixteenth = s16;
                skipMute      = rng.F() < sk * 0.8f;
            }
        }
        else
            skipMute = false, lastSixteenth = -1;
        skipG += ((skipMute ? 0.f : 1.f) - skipG) * 0.06f;
        float g = gateG * skipG;
        if(pu > 0.005f)
        {
            const float e = 1.f - ph;
            g *= 1.f - pu * 0.9f * e * e * e;
        }
        y[0] *= g;
        y[1] *= g;

        // ---- space: autopan / mono
        if(pa > 0.005f)
        {
            if(mono)
            {
                const float mid = 0.5f * (y[0] + y[1]);
                const float a   = pn::Min(1.f, pa * 3.f);
                y[0] += (mid - y[0]) * a;
                y[1] += (mid - y[1]) * a;
            }
            else
            {
                const float s = pn::Sin2pi(ph) * pn::Min(1.f, pa * 2.f);
                y[0] *= 1.f - pn::Max(0.f, s);
                y[1] *= 1.f + pn::Min(0.f, s);
            }
        }

        L[i] = pn::Lim(y[0]);
        R[i] = pn::Lim(y[1]);
    }
}

// ------------------------------------------------------------- buttons ---
// The three buttons as tap / hold events, robust to contact chatter: a
// button counts as pressed after 8 ms of contact and as released after
// 40 ms without. Tap = released before 1 s; hold = still down at 1 s (fires
// then). A press is cancelled (no event) while the mode menu is open or
// while MODE and LOAD are down together (the Parasites menu combo).
// Index: 0 FREEZE, 1 MODE, 2 LOAD.
struct Buttons
{
    bool st[3]       = {false, false, false};
    int  pressT[3]   = {0, 0, 0};
    int  relT[3]     = {0, 0, 0};
    int  t[3]        = {0, 0, 0};
    bool longDone[3] = {false, false, false};
    bool cancel[3]   = {false, false, false};

    // call every 1 ms; taps / holds get a bit per button that fired
    void Tick(const bool raw[3], bool menu, uint8_t& taps, uint8_t& holds)
    {
        taps = holds = 0;
        for(int b = 0; b < 3; b++)
        {
            if(raw[b])
            {
                relT[b] = 0;
                if(!st[b] && ++pressT[b] >= 8)
                {
                    st[b]       = true;
                    t[b]        = 0;
                    longDone[b] = false;
                    cancel[b]   = menu;
                }
            }
            else
            {
                pressT[b] = 0;
                if(st[b] && ++relT[b] >= 40)
                {
                    st[b] = false;
                    if(!longDone[b] && !cancel[b])
                        taps |= (uint8_t)(1 << b);
                }
            }
        }
        const bool combo = (st[1] || raw[1]) && (st[2] || raw[2]);
        for(int b = 0; b < 3; b++)
        {
            if(!st[b])
                continue;
            if(menu || (combo && b > 0))
                cancel[b] = true;
            if(t[b] < 100000)
                t[b]++;
            if(t[b] >= 1000 && !longDone[b] && !cancel[b])
            {
                longDone[b] = true;
                holds |= (uint8_t)(1 << b);
            }
        }
    }
};

// ------------------------------------------------------- FX presets ---
// The 8 toggle FX (LOAD tap = next, FREEZE tap = on / off), as fixed,
// tempo-synced settings of the effects above:
//   0 STUTTER 1/16   1 TAPE STOP   2 FILTER SWEEP (low-pass, 2 bars)
//   3 BITCRUSH       4 WASH        5 REVERSE   6 GATE 1/16
//   7 drums: ECHO THROW (done by the drum engine) / bender: OCTAVE DOWN
static constexpr int kPresets = 8;
static float         sweepPh  = 0.f;
inline void Preset(int fx, bool drums, float beat, size_t n, Ctl& c)
{
    c.cycle = true;
    for(int b = 0; b < kButtons; b++)
        for(int k = 0; k < kKnobs; k++)
            c.amt[b][k] = 0.f, c.sgn[b][k] = 1.f;
    switch(fx)
    {
        case 0: c.amt[B_FREEZE][K_POSITION] = 0.6f; break;
        case 1: c.amt[B_FREEZE][K_SIZE] = 0.5f; break;
        case 2:
        {
            // 2 bars (8 beats) per sweep: open -> dark -> open
            sweepPh += (float)n / (pn::Max(beat, 8000.f) * 8.f);
            if(sweepPh >= 1.f)
                sweepPh -= 1.f;
            const float u = 0.5f - 0.5f * pn::Sin2pi(sweepPh + 0.25f);
            c.amt[B_FREEZE][K_DENSITY] = 0.2f + 0.6f * u;
            c.sgn[B_FREEZE][K_DENSITY] = -1.f;
            break;
        }
        case 3: c.amt[B_FREEZE][K_TEXTURE] = 0.6f; break;
        case 4: c.amt[B_FREEZE][K_BLEND] = 0.7f; break;
        case 5: c.amt[B_MODE][K_SIZE] = 1.f; break;
        case 6: c.amt[B_MODE][K_POSITION] = 0.55f; break;
        default:
            if(!drums)
                c.amt[B_WRITE][K_PITCH] = 0.8f, c.sgn[B_WRITE][K_PITCH] = -1.f;
            break;
    }
    if(fx != 2)
        sweepPh = 0.f; // the sweep always starts open
}

// for the LEDs: the strongest effect amount of a button
inline float Level(int b)
{
    float m = 0.f;
    for(int k = 0; k < kKnobs; k++)
        m = pn::Max(m, sm[b][k]);
    return m;
}
} // namespace pfx
