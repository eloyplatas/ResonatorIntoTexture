// bender_engine.h - DATA BENDER, port of DataBenderEngine.h from the Versio
// firmware (VersionBeatMidiSendTrigs) to Clouds:
//   - 32 kHz, int16 buffers in the RAM borrowed from the granular processor
//     (L in main RAM, R in CCM): 32768 samples = 1.02 s per channel (the
//     Versio has 10.9 s of float SDRAM), so the longest buffer is ~0.9 s
//   - DaisySP Svf / DelayLine / SoftClip replaced by small local versions
//   - fast math (pn.h) in the per-sample path, no loops that a NaN or an
//     infinity could keep spinning
// The firmware glue (bender_mode.cc) fills a Controls struct once per
// 32-sample block and calls Process() from the MAIN LOOP, never from the
// audio interrupt.
#pragma once
#include <stdint.h>
#include <string.h>
#include <math.h>
#include "clouds/bender/pn.h"
#include "clouds/bender/arena.h"

namespace dbe
{
enum EngineMode
{
    MACRO_TRAVERSE,
    MACRO_SILENCE,
    MICRO,
    MODE_LAST
};
enum CorruptType
{
    C_DECIMATE,
    C_DROPOUT,
    C_DESTROY,
    C_DJFILTER,
    C_VINYL,
    C_LAST
};

struct Controls
{
    float      time        = 0.5f; // 0..1
    float      repeats     = 0.f;  // 0..1
    float      mix         = 0.5f; // 0..1
    float      bend        = 0.5f; // 0..1 (Micro: noon = 1x)
    float      brk         = 0.f;  // 0..1
    float      corrupt     = 0.f;  // 0..1
    float      extraOct    = 0.f;  // added to Micro speed (V/OCT)
    int        corruptType = C_DECIMATE;
    EngineMode mode        = MACRO_TRAVERSE;
    bool       unique      = false; // per-channel random decisions
    bool       hardWindow  = false;
    bool       gate        = false; // clock input level
    bool       freeze      = false;
    bool       reverse     = false;
    uint32_t   recapture   = 0;

    float windowMs      = 10.f;
    float reverseChance = 0.f;
    float width         = 1.f;
    float breakShape    = 0.5f;
    float corruptColor  = 0.5f;
    float regen         = 0.f;
    float shuffle       = 0.f;
    float randomRepeats = 0.f;
};

struct Status
{
    volatile bool  frozen      = false;
    volatile bool  reverse     = false;
    volatile bool  capturing   = false;
    volatile bool  clocked     = false;
    volatile float periodFlash = 0.f;
    volatile float sliceFlash  = 0.f;
    volatile float outLevel    = 0.f;
};
static Status status;

// --------------------------------------------------------------- buffer ----
static constexpr uint32_t kRingBits = 15;
static constexpr uint32_t kRing     = 1u << kRingBits; // 32768 = 1.02 s
static constexpr uint32_t kMask     = kRing - 1;
static constexpr uint32_t kSafety   = 1024;
static constexpr float    kMaxT     = 0.9f; // longest buffer (s)
static int16_t*           ring[2]   = {nullptr, nullptr};

static float sr = 32000.f;

static uint32_t rngState = 0x9E3779B9u;
inline uint32_t Rnd()
{
    rngState ^= rngState << 13;
    rngState ^= rngState >> 17;
    rngState ^= rngState << 5;
    return rngState;
}
inline float Frand() { return (Rnd() >> 8) * (1.f / 16777216.f); }

// DaisySP Svf (double-sampled Chamberlin), the parts used here
struct Svf
{
    float freq = 0.25f, damp = 1.f, res = 0.f, fc = 1000.f;
    float low = 0.f, high = 0.f, band = 0.f, outLow = 0.f, outHigh = 0.f;
    void  Reset() { low = high = band = outLow = outHigh = 0.f; }
    void  Update()
    {
        freq = 2.f * pn::Sin(3.1415927f * pn::Min(0.25f, fc / (sr * 2.f)));
        damp = pn::Min(2.f * (1.f - pn::Pow(2.f, 0.25f * log2f(pn::Max(res, 1e-6f)))),
                       pn::Min(2.f, 2.f / freq - freq * 0.5f));
    }
    void SetFreq(float f)
    {
        fc = pn::Clamp(f, 1.f, sr / 3.f);
        Update();
    }
    void SetRes(float r)
    {
        res = pn::Clamp(r, 0.f, 1.f);
        Update();
    }
    void Process(float in)
    {
        float n;
        // first pass
        n    = in - damp * band;
        low  = low + freq * band;
        high = n - low;
        band = freq * high + band;
        outLow  = 0.5f * low;
        outHigh = 0.5f * high;
        // second pass
        n    = in - damp * band;
        low  = low + freq * band;
        high = n - low;
        band = freq * high + band;
        outLow += 0.5f * low;
        outHigh += 0.5f * high;
    }
    float Low() const { return outLow; }
    float High() const { return outHigh; }
};

// ----------------------------------------------------------------- state ---
struct Voice
{
    uint32_t base   = 0;
    float    len    = 1.f;
    float    phase  = 0.f;
    float    speed  = 1.f;
    float    slope  = 0.f;
    bool     live   = true;
    bool     mute   = false;
    uint32_t muteAt = 0;
    float    sign   = 1.f;
    float    gain   = 1.f;
};

struct Ratio
{
    uint8_t num, den;
};
static const Ratio kMacroRatios[8]
    = {{1, 4}, {1, 3}, {1, 2}, {1, 1}, {2, 1}, {3, 1}, {4, 1}, {8, 1}};
static const Ratio kMicroRatios[8]
    = {{1, 16}, {1, 12}, {1, 8}, {1, 6}, {1, 4}, {1, 3}, {1, 2}, {1, 1}};
static const uint8_t kMacroRepeats[8]  = {1, 2, 3, 4, 6, 8, 12, 16};
static const uint8_t kMicroRepeats[10] = {1, 2, 3, 4, 6, 8, 12, 16, 24, 32};

struct CorruptParams
{
    float holdRate  = 1.f;
    float quant     = 32768.f;
    float dropProb  = 0.f;
    float dropAmt   = 0.f;
    float drive     = 1.f;
    float destroyMx = 0.f;
    float djD       = 0.f;
    float vinylAmt  = 0.f;
    float wowDepth  = 0.f;
    float dropLen   = 1.f;
    float foldMix   = 1.f;
    float djRes     = 0.35f;
    float popMul    = 1.f;
};

static Voice voice[2];

static EngineMode mode       = MACRO_TRAVERSE;
static bool       uniqueSt   = false;
static bool       hardWindow = false;

static uint32_t writePos    = 0;
static uint32_t S           = 0;
static uint32_t T           = 16000;
static uint32_t N           = 1;
static uint32_t sliceLen    = 16000;
static uint32_t sliceIdx    = 0;
static uint32_t t           = 0;
static uint32_t tInSlice    = 0;
static uint32_t curSliceLen = 16000;

static bool     frozen       = false;
static uint32_t frozenS      = 0;
static uint32_t frozenLen    = 0;
static bool     reverse      = false;
static bool     capturing    = false;
static uint32_t captureEnd   = 0;
static uint32_t recaptureAck = 0;

static float    targetT     = 16000.f;
static uint32_t targetN     = 1;
static float    bendAmt     = 0.f;
static float    microSpeed  = 1.f;
static float    breakAmt    = 0.f;
static int      corruptType = C_DECIMATE;
static float    mixPrev = 0.f, mixNow = 0.f;

static uint32_t sampleClock = 0;
static uint32_t lastEdge    = 0;
static uint32_t clockPeriod = 0;
static bool     haveEdge    = false;
static bool     lastGate    = false;
static uint32_t edgesSince  = 0;
static bool     clocked     = false;
static bool     resyncReq   = false;
static int      clockIdx    = 3;

static float windowMs      = 10.f;
static float revChance     = 0.f;
static float width         = 1.f;
static float breakShape    = 0.5f;
static float regen         = 0.f;
static float shuffle       = 0.f;
static float randomRepeats = 0.f;
static float fbWet[2]      = {0.f, 0.f};

static CorruptParams cp;

// --------------------------------------------------------------- corrupt ---
struct Dropper
{
    uint32_t remain = 0;
    float    g      = 1.f;
    float    Process()
    {
        float target = 1.f;
        if(remain > 0)
        {
            remain--;
            target = 0.f;
        }
        else if(Frand() < cp.dropProb)
        {
            remain = (uint32_t)((0.004f + Frand() * (0.02f + 0.12f * cp.dropAmt))
                                * cp.dropLen * sr);
        }
        g += (target - g) * 0.08f;
        return g;
    }
};

static constexpr float kPopLevel = 0.12f;

// wow delay line: int16, 1024 samples (32 ms) from the arena
static constexpr uint32_t kWow = 1024, kWowM = kWow - 1;

inline float SoftClip(float x)
{
    if(x < -3.f)
        return -1.f;
    if(x > 3.f)
        return 1.f;
    return x * (27.f + x * x) / (27.f + 9.f * x * x);
}

struct Corruptor
{
    Svf      dj, vlp, vhp;
    int16_t* wow  = nullptr;
    uint32_t wowW = 0;
    float    holdPh = 0.f, held = 0.f;
    float    crackle = 0.f, crackleLp = 0.f;

    void Init()
    {
        dj.Reset(), vlp.Reset(), vhp.Reset();
        dj.SetRes(0.35f);
        vlp.SetRes(0.1f);
        vhp.SetRes(0.1f);
        holdPh = held = crackle = crackleLp = 0.f;
    }

    void SetBlock()
    {
        const float d = cp.djD;
        if(d < 0.f)
            dj.fc = 20000.f * pn::Pow(0.002f, -d); // 20 kHz -> 40 Hz
        else
            dj.fc = 20.f * pn::Pow(400.f, d); // 20 Hz -> 8 kHz
        dj.fc = pn::Clamp(dj.fc, 1.f, sr / 3.f);
        dj.res = cp.djRes;
        dj.Update();
        vlp.SetFreq(15000.f - cp.vinylAmt * 11500.f);
        vhp.SetFreq(25.f + cp.vinylAmt * 280.f);
    }

    static inline float Fold(float x)
    {
        x = x * 0.25f + 0.25f;
        x -= pn::Floor(x);
        return 1.f - 4.f * fabsf(x - 0.5f);
    }

    float Process(float x, int type, float dropGain, float wowLfo)
    {
        dj.Process(x); // always run so switching types doesn't click
        switch(type)
        {
            case C_DECIMATE:
            {
                holdPh += cp.holdRate;
                if(holdPh >= 1.f)
                {
                    holdPh -= 1.f;
                    held = x;
                }
                return pn::Floor(held * cp.quant + 0.5f) / cp.quant;
            }
            case C_DROPOUT: return x * dropGain;
            case C_DESTROY:
            {
                const float d  = x * cp.drive;
                const float cl = SoftClip(d);
                float y = SoftClip((cl + (Fold(d) - cl) * cp.foldMix) * 0.9f);
                return x + (y - x) * cp.destroyMx;
            }
            case C_DJFILTER:
            {
                if(fabsf(cp.djD) < 0.03f)
                    return x;
                return cp.djD < 0.f ? dj.Low() : dj.High();
            }
            case C_VINYL:
            default:
            {
                const float a = cp.vinylAmt;
                wow[wowW & kWowM] = pn::F2S(x * 0.5f);
                const float d = pn::Clamp((0.005f + cp.wowDepth * wowLfo) * sr,
                                          1.f, (float)(kWow - 4));
                float y = 2.f * pn::ReadFracS(wow, kWowM, wowW, d);
                wowW++;
                vlp.Process(y);
                y = vlp.Low();
                vhp.Process(y);
                y = vhp.High();
                if(Frand() < a * a * 0.0025f * cp.popMul)
                {
                    const float r = Frand();
                    crackle = (Frand() < 0.5f ? -1.f : 1.f) * a * kPopLevel
                              * (0.15f + 0.85f * r * r * r);
                }
                crackleLp += 0.35f * (crackle - crackleLp);
                crackle *= 0.6f;
                y += crackleLp + (Frand() * 2.f - 1.f) * a * 0.006f;
                y = SoftClip(y * (1.f + a)) / (1.f + 0.4f * a);
                float m = a * 8.f;
                m       = m > 1.f ? 1.f : m;
                return x + (y - x) * m;
            }
        }
    }
};

static Corruptor corr[2];
static Dropper   drop[2];
static float     wowPhase = 0.f; // 0..1

// --------------------------------------------------------------- helpers ---
static constexpr float kS2F = 1.f / 32767.f;
inline float ReadRing(const int16_t* buf, uint32_t base, float pos)
{
    const uint32_t ip = (uint32_t)pos;
    const float    f  = pos - (float)ip;
    const float    a  = (float)buf[(base + ip) & kMask];
    const float    b  = (float)buf[(base + ip + 1) & kMask];
    return (a + (b - a) * f) * kS2F;
}

// no while loops: a NaN or infinity must not hang the main loop
inline float WrapPhase(float p, float len)
{
    if(!(p == p) || p > 1e9f || p < -1e9f)
        return 0.f;
    if(p >= len)
    {
        p -= len;
        if(p >= len)
            p -= len * pn::Floor(p / len);
    }
    else if(p < 0.f)
    {
        p += len;
        if(p < 0.f)
            p -= len * pn::Floor(p / len);
    }
    return (p >= 0.f && p < len) ? p : 0.f;
}

inline float Limit(float x)
{
    const float ax = fabsf(x);
    if(ax < 0.9f)
        return x;
    const float y = 0.9f + 0.1f * pn::Tanh((ax - 0.9f) * 10.f);
    return x < 0.f ? -y : y;
}

inline void DecideVoice(Voice& v)
{
    const float L     = (float)sliceLen;
    const bool  micro = (mode == MICRO);
    float       speed = 1.f;
    float       slope = 0.f;
    bool        mute  = false;
    uint32_t    back  = 0;
    uint32_t    fwd   = 0;
    bool        bent  = false;

    if(!micro)
    {
        if(breakAmt > 0.02f && Frand() < breakAmt)
        {
            if(mode == MACRO_TRAVERSE)
            {
                if(frozen)
                {
                    uint32_t cnt = frozenLen / sliceLen;
                    fwd          = (Rnd() % (cnt > 0 ? cnt : 1)) * sliceLen;
                }
                else
                {
                    uint32_t maxK = 1 + (uint32_t)(breakShape * 14.f);
                    back          = (1 + Rnd() % maxK) * sliceLen;
                }
            }
            else
                mute = Frand() < 0.85f;
        }
        if(bendAmt > 0.02f && Frand() < bendAmt)
        {
            bent      = true;
            int types = 2 + (int)(bendAmt * 4.99f);
            if(types > 6)
                types = 6;
            switch(Rnd() % types)
            {
                case 0: speed = -1.f; break;
                case 1: speed = 0.5f; break;
                case 2: slope = -1.f / L; break;
                case 3: speed = 2.f; break;
                case 4: speed = -0.5f; break;
                default:
                    speed = 0.f;
                    slope = 1.f / L;
                    break;
            }
        }
    }
    else
    {
        if(breakAmt < 0.5f)
        {
            float u = breakAmt * 2.f;
            if(u > 0.02f)
            {
                if(frozen)
                    fwd = (uint32_t)(u * (float)(frozenLen > sliceLen
                                                     ? frozenLen - sliceLen
                                                     : 0));
                else
                {
                    float maxBack = 16.f * (float)T * pn::Max(0.05f, breakShape * 2.f);
                    float lim     = (float)(kRing - T - kSafety);
                    if(maxBack > lim)
                        maxBack = lim;
                    back = (uint32_t)(u * maxBack);
                }
            }
        }
        else if(Frand() < (breakAmt - 0.5f) * 1.8f)
            mute = true;
        bent = fabsf(microSpeed - 1.f) > 0.002f;
    }

    const bool revSlice = revChance > 0.001f && Frand() < revChance;
    if(revSlice)
        bent = true;

    const bool clean = !bent && !reverse && slope == 0.f;

    uint32_t muteAt = 0;
    if(mute && breakShape > 0.5f)
        muteAt = (uint32_t)((breakShape - 0.5f) * 1.5f * L);

    v.live   = false;
    v.len    = L;
    v.speed  = speed;
    v.slope  = slope;
    v.mute   = mute;
    v.muteAt = muteAt;
    v.sign   = revSlice ? -1.f : 1.f;
    v.phase  = 0.f;

    if(shuffle > 0.001f && Frand() < shuffle && back == 0 && fwd == 0
       && (sliceIdx > 0 || !clean || frozen))
    {
        if(frozen)
        {
            uint32_t cnt = frozenLen / sliceLen;
            v.base       = S + (Rnd() % (cnt > 0 ? cnt : 1)) * sliceLen;
        }
        else
            v.base = S - T + (Rnd() % N) * sliceLen;
    }
    else if(frozen)
        v.base = S + fwd;
    else if(back == 0)
    {
        if(sliceIdx == 0)
        {
            if(clean)
            {
                v.live = true;
                v.base = S;
            }
            else
                v.base = S - sliceLen;
        }
        else
            v.base = S;
    }
    else
    {
        if(sliceIdx == 0 && back < sliceLen)
            back = sliceLen;
        uint32_t lim = kRing - T - kSafety;
        if(back > lim)
            back = (lim / sliceLen) * sliceLen;
        v.base = S - back;
    }
}

inline void StartSlice()
{
    uint32_t remaining = T - t;
    curSliceLen        = remaining < sliceLen ? remaining : sliceLen;
    tInSlice           = 0;
    DecideVoice(voice[0]);
    if(uniqueSt)
        DecideVoice(voice[1]);
    else
    {
        float g       = voice[1].gain;
        voice[1]      = voice[0];
        voice[1].gain = g;
    }
    status.sliceFlash = 1.f;
}

inline void StartPeriod()
{
    float tt = targetT;
    if(frozen && tt > (float)frozenLen)
        tt = (float)frozenLen;
    T = (uint32_t)tt;
    N = targetN;
    if(N > 1 && randomRepeats > 0.001f && Frand() < randomRepeats)
        N = 1 + Rnd() % N;
    if(T < 64)
        T = 64;
    sliceLen = T / N;
    if(sliceLen < 32)
        sliceLen = 32;
    t        = 0;
    sliceIdx = 0;
    S        = frozen ? frozenS : writePos;
    StartSlice();
    status.periodFlash = 1.f;
}

inline void OnClockEdge()
{
    if(haveEdge)
    {
        uint32_t per = sampleClock - lastEdge;
        if(per > (uint32_t)(0.01f * sr) && per < (uint32_t)(10.f * sr))
            clockPeriod = per;
    }
    haveEdge = true;
    lastEdge = sampleClock;

    const Ratio& r
        = (mode == MICRO) ? kMicroRatios[clockIdx] : kMacroRatios[clockIdx];
    edgesSince++;
    if(edgesSince >= r.num)
    {
        edgesSince = 0;
        resyncReq  = true;
    }
}

// ------------------------------------------------------ block controls -----
inline void ApplyControls(const Controls& c)
{
    mode             = c.mode;
    uniqueSt         = c.unique;
    hardWindow       = c.hardWindow;
    windowMs         = c.windowMs;
    revChance        = c.reverseChance;
    width            = c.width;
    breakShape       = c.breakShape;
    regen            = c.regen;
    shuffle          = c.shuffle;
    randomRepeats    = c.randomRepeats;
    const bool micro = (mode == MICRO);

    if(c.gate && !lastGate)
        OnClockEdge();
    lastGate = c.gate;
    clocked  = haveEdge && clockPeriod > 0
              && (sampleClock - lastEdge) < (uint32_t)(clockPeriod * 2.5f)
              && (sampleClock - lastEdge) < (uint32_t)(10.f * sr);

    // TIME
    const float maxT = kMaxT * sr;
    if(clocked)
    {
        float pos = c.time * 7.999f;
        if(fabsf(pos - ((float)clockIdx + 0.5f)) > 0.65f)
            clockIdx = (int)pos;
        const Ratio& r = micro ? kMicroRatios[clockIdx] : kMacroRatios[clockIdx];
        targetT        = (float)clockPeriod * r.num / r.den;
        // Clouds holds 0.9 s: halve a too long clocked buffer until it fits
        for(int k = 0; k < 8 && targetT > maxT; k++)
            targetT *= 0.5f;
    }
    else
    {
        const float tmin = micro ? 0.005f : 0.05f;
        const float tmax = micro ? 0.4f : kMaxT;
        targetT          = tmin * pn::Exp2(c.time * log2f(tmax / tmin)) * sr;
    }
    if(targetT > maxT)
        targetT = maxT;

    // REPEATS
    targetN = micro ? kMicroRepeats[(int)(pn::Clamp(c.repeats, 0.f, 1.f) * 9.999f)]
                    : kMacroRepeats[(int)(pn::Clamp(c.repeats, 0.f, 1.f) * 7.999f)];

    mixPrev = mixNow;
    mixNow  = pn::Clamp(c.mix, 0.f, 1.f);

    // BEND
    bendAmt = c.bend;
    {
        float oct = (c.bend - 0.5f) * 5.f;
        if(fabsf(oct) < 0.03f)
            oct = 0.f;
        oct += c.extraOct;
        if(fabsf(oct) < 0.003f)
            oct = 0.f;
        microSpeed = pn::Exp2(pn::Clamp(oct, -4.f, 4.f));
    }

    breakAmt = c.brk;

    // CORRUPT
    corruptType     = c.corruptType;
    const float a   = c.corrupt;
    const float col = c.corruptColor;
    const float aRate = a * pn::Min(1.f, 2.f * (1.f - col));
    const float aBits = a * pn::Min(1.f, 2.f * col);
    cp.holdRate  = 1.f - aRate * aRate * 0.985f;
    cp.quant     = pn::Exp2(15.f - aBits * 12.f);
    cp.dropProb  = a < 0.01f ? 0.f : (a * a * 30.f) / sr;
    cp.dropAmt   = a;
    cp.drive     = (1.f + a * a * 30.f) * (col > 0.5f ? 1.f + (col - 0.5f) * 4.f : 1.f);
    cp.foldMix   = pn::Min(1.f, col * 2.f);
    cp.dropLen   = pn::Exp2((col - 0.5f) * 4.f);
    cp.djRes     = pn::Clamp(0.35f + (col - 0.5f) * 1.2f, 0.f, 0.95f);
    cp.popMul    = 2.f * col;
    cp.destroyMx = a * 4.f > 1.f ? 1.f : a * 4.f;
    cp.djD       = (a - 0.5f) * 2.f;
    cp.vinylAmt  = a;
    cp.wowDepth  = a * 0.0025f * 2.f * (1.f - col);
    corr[0].SetBlock();
    corr[1].SetBlock();

    reverse = c.reverse;

    if(c.recapture != recaptureAck)
    {
        recaptureAck = c.recapture;
        edgesSince   = 0;
        if(c.freeze)
        {
            frozen     = false;
            capturing  = true;
            captureEnd = writePos + (uint32_t)targetT;
        }
        else
            capturing = false;
        StartPeriod();
    }

    if(capturing)
    {
        if(!c.freeze)
            capturing = false;
        else if((int32_t)(writePos - captureEnd) >= 0)
        {
            capturing = false;
            frozenLen = (uint32_t)targetT;
            frozenS   = writePos - frozenLen;
            frozen    = true;
            StartPeriod();
        }
    }
    else if(c.freeze != frozen)
    {
        if(c.freeze)
        {
            frozenLen = T;
            frozenS   = writePos - T;
            frozen    = true;
            StartPeriod();
        }
        else
            frozen = false;
    }

    if(resyncReq)
    {
        resyncReq = false;
        StartPeriod();
    }

    status.frozen    = frozen;
    status.reverse   = reverse;
    status.capturing = capturing;
    status.clocked   = clocked;
}

// ---------------------------------------------------------------- public ---
// buffers from the arena (zeroed there): call when the mode starts
inline void Attach()
{
    // L in main RAM, R in CCM (the CCM block holds exactly one ring)
    ring[0]     = arena::M.Take<int16_t>(kRing);
    ring[1]     = arena::C.Take<int16_t>(kRing);
    corr[0].wow = arena::M.Take<int16_t>(kWow);
    corr[1].wow = arena::M.Take<int16_t>(kWow);
}

// fresh state (also after a NaN): keeps the buffers' content
inline void Reset()
{
    for(int c = 0; c < 2; c++)
    {
        corr[c].Init();
        voice[c]    = Voice();
        fbWet[c]    = 0.f;
        drop[c]     = Dropper();
    }
    frozen = capturing = false;
    mixPrev = mixNow;
    StartPeriod();
}

inline void Init(float samplerate, uint32_t seed)
{
    sr = samplerate;
    rngState ^= seed * 2654435761u + 1u;
    if(rngState == 0)
        rngState = 0x9E3779B9u;
}

// in / out: 0..1 floats, one block
inline void Process(const Controls& c,
                    const float*    inL_,
                    const float*    inR_,
                    float*          outL,
                    float*          outR,
                    size_t          size)
{
    ApplyControls(c);

    const bool  micro   = (mode == MICRO);
    const bool  twoTap  = micro && !hardWindow;
    const float revSign = reverse ? -1.f : 1.f;
    const float wowInc  = 0.55f / sr;
    float       peak    = 0.f;

    for(size_t i = 0; i < size; i++)
    {
        const float inL = inL_[i];
        const float inR = inR_[i];

        if(!frozen)
        {
            if(regen > 0.001f)
            {
                ring[0][writePos & kMask] = pn::F2S(Limit(inL + regen * fbWet[0]));
                ring[1][writePos & kMask] = pn::F2S(Limit(inR + regen * fbWet[1]));
            }
            else
            {
                ring[0][writePos & kMask] = pn::F2S(inL);
                ring[1][writePos & kMask] = pn::F2S(inR);
            }
        }

        float       wN    = (float)curSliceLen * 0.25f;
        const float wBase = windowMs * 0.001f * sr;
        float       W     = hardWindow ? pn::Max(8.f, wBase * 0.05f) : wBase;
        if(W > wN)
            W = wN;
        if(W < 1.f)
            W = 1.f;
        float win;
        {
            const float iw = 1.f / W;
            float       a  = (float)tInSlice * iw;
            float       b  = (float)(curSliceLen - tInSlice) * iw;
            win            = a < b ? a : b;
            if(win > 1.f)
                win = 1.f;
        }

        float wet[2];
        for(int ch = 0; ch < 2; ch++)
        {
            Voice& v = voice[ch];
            float  s;
            if(v.live)
                // the live slice plays the write head: the input itself
                // (not its 16-bit copy)
                s = ch ? inR : inL;
            else
            {
                float sp = (micro ? microSpeed : v.speed) * revSign * v.sign;
                if(twoTap)
                {
                    float p2 = WrapPhase(v.phase + v.len * 0.5f, v.len);
                    float x  = v.phase / v.len;
                    float g1 = 1.f - fabsf(2.f * x - 1.f);
                    s        = ReadRing(ring[ch], v.base, v.phase) * g1
                        + ReadRing(ring[ch], v.base, p2) * (1.f - g1);
                }
                else
                    s = ReadRing(ring[ch], v.base, v.phase);
                v.phase = WrapPhase(v.phase + sp, v.len);
                v.speed += v.slope;
            }
            const float gt = (v.mute && tInSlice >= v.muteAt) ? 0.f : 1.f;
            v.gain += (gt - v.gain) * 0.01f;
            const bool needWin = !(v.live && N == 1);
            wet[ch]            = s * v.gain * (needWin ? win : 1.f);
        }

        wowPhase += wowInc;
        if(wowPhase >= 1.f)
            wowPhase -= 1.f;
        const float wowLfo = pn::Sin2pi(wowPhase)
                             + 0.3f * pn::Sin2pi(3.1f * wowPhase + 0.159155f);
        float dg0 = 1.f, dg1 = 1.f;
        if(corruptType == C_DROPOUT)
        {
            dg0 = drop[0].Process();
            dg1 = uniqueSt ? drop[1].Process() : dg0;
        }
        wet[0] = corr[0].Process(wet[0], corruptType, dg0, wowLfo);
        wet[1] = corr[1].Process(wet[1], corruptType, dg1, wowLfo);

        if(fabsf(width - 1.f) > 0.01f)
        {
            const float mid  = 0.5f * (wet[0] + wet[1]);
            const float side = 0.5f * (wet[0] - wet[1]) * width;
            wet[0]           = mid + side;
            wet[1]           = mid - side;
        }

        fbWet[0] = voice[0].live ? 0.f : wet[0];
        fbWet[1] = voice[1].live ? 0.f : wet[1];

        // equal-power mix, interpolated across the block
        const float m  = mixPrev + (mixNow - mixPrev) * ((float)i / (float)size);
        const float dG = pn::Sin2pi(m * 0.25f + 0.25f); // cos(m pi/2)
        const float wG = pn::Sin2pi(m * 0.25f);         // sin(m pi/2)
        const float oL = Limit(inL * dG + wet[0] * wG);
        const float oR = Limit(inR * dG + wet[1] * wG);
        outL[i]        = oL;
        outR[i]        = oR;
        peak           = pn::Max(peak, pn::Max(fabsf(oL), fabsf(oR)));

        if(!frozen)
            writePos++;
        sampleClock++;
        t++;
        tInSlice++;
        if(t >= T)
            StartPeriod();
        else if(tInSlice >= curSliceLen)
        {
            sliceIdx++;
            StartSlice();
        }
    }
    status.outLevel = peak;
}

} // namespace dbe
