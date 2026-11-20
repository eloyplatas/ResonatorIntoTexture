// pn.h - what the front panel hands to the engines, plus small shared DSP
// helpers. Port of Panel.h from VersionBeatMidiSendTrigs (Versio / Daisy)
// to Clouds (STM32F405, Cortex-M4F at 168 MHz, 32 kHz):
//   - no SDRAM: delay lines are int16 and live in the shared RAM arenas
//     (arena.h), only one engine runs at a time
//   - fast sine / tanh approximations: the M4 has no fast libm
//
// Knobs are 0..1 (knob + CV), row / col = submode, a = toggle (double-tap
// or FREEZE gate), action counts "hold 0.6..4 s + release" gestures.
#pragma once
#include <stdint.h>
#include <stddef.h>
#include <math.h>

// k = layer 1 values, s = layer 2 values (Versio numbering K0..K6)
struct Panel
{
    float    k[7] = {0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f};
    float    s[7] = {0.5f, 0.5f, 0.f, 0.5f, 0.5f, 0.5f, 0.5f};
    int      row    = 0;
    int      col    = 0;
    bool     gate   = false;
    bool     a      = false;
    uint32_t action = 0;
};

// 4 RGB LEDs as the Versio engines paint them (converted to the Clouds
// red / green LEDs by the firmware)
struct LedOut
{
    float c[4][3];
    void  Set(int i, float r, float g, float b)
    {
        c[i][0] = r;
        c[i][1] = g;
        c[i][2] = b;
    }
};

namespace pn
{
static constexpr float kSr = 32000.f; // Clouds codec rate

inline float Clamp(float x, float lo, float hi)
{
    return x < lo ? lo : (x > hi ? hi : x);
}

inline float Min(float a, float b) { return a < b ? a : b; }
inline float Max(float a, float b) { return a > b ? a : b; }

// floor without the libm call (the M4 has no rounding instruction)
inline float Floor(float x)
{
    const float t = (float)(int32_t)x;
    return t > x ? t - 1.f : t;
}

// 2^x (exponent bits + Taylor polynomial around 0, |f| <= 0.5): max
// relative error ~5e-5, and exact to the last bits for small x, where the
// decay coefficients (0.9999...) live
inline float Exp2(float x)
{
    if(x < -126.f)
        return 0.f;
    if(x > 126.f)
        x = 126.f;
    const float fl = Floor(x + 0.5f);
    const float f  = x - fl;
    const float p
        = 1.f
          + f * (0.6931472f + f * (0.2402265f + f * (0.0555041f + f * (0.0096181f + f * 0.0013334f))));
    union
    {
        float    f;
        int32_t  i;
    } u;
    u.f = p;
    u.i += (int32_t)fl << 23;
    return u.f;
}
// e^x
inline float Exp(float x) { return Exp2(x * 1.4426950f); }
// a^x for a constant a (log2f of a constant folds at compile time)
inline float Pow(float a, float x) { return Exp2(x * log2f(a)); }

inline float OnePole(float hz, float sr)
{
    const float c = 1.f - Exp(-6.2831853f * hz / sr);
    return c > 1.f ? 1.f : c;
}

// sin(2 pi ph), any ph (max error ~0.001)
inline float Sin2pi(float ph)
{
    float x = ph - Floor(ph + 0.5f); // -0.5..0.5
    float y = 8.f * x - 16.f * x * fabsf(x);
    return 0.225f * (y * fabsf(y) - y) + y;
}
// sin(x), x in radians
inline float Sin(float x) { return Sin2pi(x * 0.15915494f); }
inline float Cos2pi(float ph) { return Sin2pi(ph + 0.25f); }
// tan(x) for 0 <= x <= 1.42 (filter prewarp), Pade 7/6
inline float Tan(float x)
{
    const float x2 = x * x;
    return x * (135135.f - x2 * (17325.f - x2 * (378.f - x2)))
           / (135135.f - x2 * (62370.f - x2 * (3150.f - 28.f * x2)));
}

// tanh, rational approximation (exact limits, error < 0.003)
inline float Tanh(float x)
{
    if(x > 4.97f)
        return 1.f;
    if(x < -4.97f)
        return -1.f;
    const float x2 = x * x;
    const float a  = x * (135135.f + x2 * (17325.f + x2 * (378.f + x2)));
    const float b  = 135135.f + x2 * (62370.f + x2 * (3150.f + x2 * 28.f));
    return a / b;
}

// DaisySP SoftClip
inline float SoftClip(float x)
{
    if(x < -3.f)
        return -1.f;
    if(x > 3.f)
        return 1.f;
    return x * (27.f + x * x) / (27.f + 9.f * x * x);
}

// transparent below 0.9, soft ceiling at 1
inline float Lim(float x)
{
    const float ax = fabsf(x);
    if(ax < 0.9f)
        return x;
    const float y = 0.9f + 0.1f * Tanh((ax - 0.9f) * 10.f);
    return x < 0.f ? -y : y;
}

// transparent below |1|, soft limit towards |2| (for feedback loops)
inline float LoopClip(float x)
{
    if(x > 1.f)
        return 2.f - 1.f / x;
    if(x < -1.f)
        return -2.f - 1.f / x;
    return x;
}

// ---- float delay lines
inline float ReadFrac(const float* buf, uint32_t mask, uint32_t wp, float d)
{
    const uint32_t di = (uint32_t)d;
    const float    f  = d - (float)di;
    const float    a  = buf[(wp - di) & mask];
    const float    b  = buf[(wp - di - 1) & mask];
    return a + (b - a) * f;
}

// ---- int16 delay lines (full scale = +-1.0)
static constexpr float kS2F = 1.f / 32767.f;
inline int16_t F2S(float x)
{
    x = x * 32767.f;
    if(x > 32767.f)
        x = 32767.f;
    if(x < -32767.f)
        x = -32767.f;
    return (int16_t)(int32_t)(x < 0.f ? x - 0.5f : x + 0.5f);
}
inline float ReadFracS(const int16_t* buf, uint32_t mask, uint32_t wp, float d)
{
    const uint32_t di = (uint32_t)d;
    const float    f  = d - (float)di;
    const float    a  = (float)buf[(wp - di) & mask];
    const float    b  = (float)buf[(wp - di - 1) & mask];
    return (a + (b - a) * f) * kS2F;
}
inline float ReadAtS(const int16_t* buf, uint32_t mask, uint32_t base, float pos)
{
    const uint32_t ip = (uint32_t)pos;
    const float    f  = pos - (float)ip;
    const float    a  = (float)buf[(base + ip) & mask];
    const float    b  = (float)buf[(base + ip + 1) & mask];
    return (a + (b - a) * f) * kS2F;
}

struct Rng
{
    uint32_t s = 0x6C8E9CF5u;
    uint32_t Next()
    {
        s ^= s << 13;
        s ^= s >> 17;
        s ^= s << 5;
        return s;
    }
    float F() { return (Next() >> 8) * (1.f / 16777216.f); } // 0..1
    float B() { return F() * 2.f - 1.f; }                     // -1..1
};

// gate clock: period measurement, "clocked" detection
struct Clock
{
    uint32_t n = 0, last = 0, period = 0;
    bool     have = false, lastGate = false;
    bool     Tick(bool gate, uint32_t samples, float sr) // true on rising edge
    {
        bool edge = gate && !lastGate;
        lastGate  = gate;
        if(edge)
        {
            const uint32_t per = n - last;
            if(have && per > (uint32_t)(0.01f * sr) && per < (uint32_t)(10.f * sr))
                period = per;
            have = true;
            last = n;
        }
        n += samples;
        return edge;
    }
    bool Clocked(float sr) const
    {
        return have && period > 0 && (n - last) < (uint32_t)(period * 2.5f)
               && (n - last) < (uint32_t)(10.f * sr);
    }
};

inline float Tri(float x) { return 1.f - fabsf(2.f * x - 1.f); }

// reverse playback of a delay line in chunks of T samples: two heads half a
// chunk apart, triangle windows (sum = 1). Read delays go up to 2T + 1.
struct RevReader
{
    float p = 0.f;
    void  Step(float T, float& d1, float& g1, float& d2, float& g2)
    {
        p += 1.f;
        if(p >= T)
            p -= T;
        float p2 = p + T * 0.5f;
        if(p2 >= T)
            p2 -= T;
        d1 = 2.f * p + 1.f;
        d2 = 2.f * p2 + 1.f;
        g1 = Tri(p / T);
        g2 = Tri(p2 / T);
    }
};

// quantized zone from a 0..1 knob with hysteresis
inline int Zone(float k, int n, int& cur)
{
    const float pos = k * (float)n;
    if(fabsf(pos - ((float)cur + 0.5f)) > 0.6f)
    {
        int z = (int)pos;
        cur   = z >= n ? n - 1 : (z < 0 ? 0 : z);
    }
    return cur;
}

} // namespace pn
