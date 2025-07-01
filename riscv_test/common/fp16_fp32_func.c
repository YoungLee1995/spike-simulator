#include "fp16_fp32_func.h"
#include <stdint.h>
#include <math.h>
#include <limits.h>

typedef unsigned short Half;
typedef unsigned short ushort;
typedef unsigned int uint;


uint as_uint(const float x)
{
    suf32 val;
    val.f = x;
    return val.u;
}

float as_float(const uint x)
{
    suf32 val;
    val.u = x;
    return val.f;
}

float half_to_float(Half h)
{
    uint sign = (h >> 15) & 0x1;
    uint exp = (h >> 10) & 0x1F;
    uint frac = h & 0x3FF;

    uint f;

    if (exp == 0)
    {
        if (frac == 0)
        {
            f = sign << 31; // Zero
        }
        else
        {
            // Subnormal number
            exp = 1;
            while ((frac & 0x400) == 0)
            {
                frac <<= 1;
                exp--;
            }
            frac &= 0x3FF;
            exp = exp + 127 - 15;
            f = (sign << 31) | (exp << 23) | (frac << 13);
        }
    }
    else if (exp == 0x1F)
    {
        // Inf or NaN
        f = (sign << 31) | 0x7F800000 | (frac << 13);
    }
    else
    {
        // Normalized
        exp = exp + 127 - 15;
        f = (sign << 31) | (exp << 23) | (frac << 13);
    }

    return as_float(f);
}

Half float_to_half(float f)
{
    uint x = as_uint(f);

    uint sign = (x >> 31) & 0x1;
    int exp = ((x >> 23) & 0xFF) - 127 + 15;
    uint frac = x & 0x7FFFFF;

    Half h;

    if (exp <= 0)
    {
        // Subnormal or underflow
        if (exp < -10)
            h = sign << 15;
        else
        {
            frac |= 0x800000;
            int shift = 14 - exp;
            frac = frac >> shift;
            h = (sign << 15) | (frac >> 13);
        }
    }
    else if (exp >= 0x1F)
    {
        // Overflow to Inf
        h = (sign << 15) | 0x7C00;
    }
    else
    {
        h = (sign << 15) | (exp << 10) | (frac >> 13);
    }

    return h;
}

void float_to_half_array(Half *dst, float *src, int size)
{
    for (int i = 0; i < size; ++i)
    {
        dst[i] = float_to_half(src[i]);
    }
}

void half_to_float_array(float *dst, Half *src, int size)
{
    for (int i = 0; i < size; ++i)
    {
        dst[i] = half_to_float(src[i]);
    }
}

// Arithmetic via float conversion
Half half_add(Half a, Half b)
{
    float fa = half_to_float(a);
    float fb = half_to_float(b);
    return float_to_half(fa + fb);
}

Half half_sub(Half a, Half b)
{
    float fa = half_to_float(a);
    float fb = half_to_float(b);
    return float_to_half(fa - fb);
}

Half half_mul(Half a, Half b)
{
    float fa = half_to_float(a);
    float fb = half_to_float(b);
    return float_to_half(fa * fb);
}

Half half_div(Half a, Half b)
{
    float fa = half_to_float(a);
    float fb = half_to_float(b);
    return float_to_half(fa / fb);
}

Half half_min(Half a, Half b)
{
    float fa = half_to_float(a);
    float fb = half_to_float(b);
    return float_to_half(fminf(fa, fb));
}

Half half_max(Half a, Half b)
{
    float fa = half_to_float(a);
    float fb = half_to_float(b);
    return float_to_half(fmaxf(fa, fb));
}
