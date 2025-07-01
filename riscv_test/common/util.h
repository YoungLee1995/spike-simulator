#ifndef _UTIL_H
#define _UTIL_H

extern void setStats(int enable);

#include <stdint.h>

#define static_assert(cond) \
    switch (0)              \
    {                       \
    case 0:                 \
    case !!(long)(cond):;   \
    }

#define get_reg(Addr) (*(volatile uint32_t *)(Addr))
#define set_reg(Addr, Value) (*(volatile uint32_t *)(Addr) = (unsigned)(Value))

#define read_reg(reg) ({unsigned long __tmp; asm volatile ("mv %0" #reg: "=r"(__tmp)); __tmp; })
#define write_reg(reg, value) ({ asm volatile("mv " #reg ", %0" : : "r"(value)); })

static int verify(int n, const volatile int *test, const int *verify)
{
    int i;
    for (i = 0; i < n / 2 * 2; i += 2)
    {
        int t0 = test[i], t1 = test[i + 1];
        int v0 = verify[i], v1 = verify[i + 1];
        if (t0 != v0)
            return i + 1;
        if (t1 != v1)
            return i + 2;
    }
    if (n % 2 != 0 && test[n - 1] != verify[n - 1])
        return n;
    return 0;
}

static int verify_double(int n, const volatile int *test, const int *verify)
{
    int i;
    for (i = 0; i < n / 2 * 2; i += 2)
    {
        int t0 = test[i], t1 = test[i + 1];
        int v0 = verify[i], v1 = verify[i + 1];
        int eq1 = (t0 == v0);
        int eq2 = (t1 == v1);
        if (!eq1 && !eq2)
            return i + 1 + (eq1 ? 0 : 1);
    }
    if (n % 2 != 0 && test[n - 1] != verify[n - 1])
        return n;
    return 0;
}

static int verify_float(int n, const volatile float *test, const float *verify)
{
    int i;
    for (i = 0; i < n / 2 * 2; i += 2)
    {
        float t0 = test[i], t1 = test[i + 1];
        float v0 = verify[i], v1 = verify[i + 1];
        int eq1 = (t0 == v0);
        int eq2 = (t1 == v1);
        if (!eq1 && !eq2)
            return i + 1 + (eq1 ? 0 : 1);
    }
    if (n % 2 != 0 && test[n - 1] != verify[n - 1])
        return n;
    return 0;
}

static void __attribute__((noinline)) barrier(int ncores)
{
    static volatile int sense;
    static volatile int count;
    static __thread int thread_sense;

    __sync_synchronize();
    thread_sense = !thread_sense;
    if (__sync_fetch_and_add(&count, 1) == ncores - 1)
    {
        count = 0;
        sense = thread_sense;
    }
    else
    {
        while (sense != thread_sense)
            ;
    }
    __sync_synchronize();
}

static uint64_t lfsr(uint64_t x)
{
    uint64_t bit = (x ^ (x >> 1)) & 1;
    x >>= 1;
    return x | (bit << 62);
}

static uintptr_t insn_len(uintptr_t pc)
{
    return (*(volatile uint32_t *)pc & 0x3) ? 4 : 2;
}

#ifdef __riscv
#include "encoding.h"
#endif

#define stringify_1(x) #x
#define stringify(x) stringify_1(x)
#define stats(code, iter)                                                             \
    do                                                                                \
    {                                                                                 \
        unsigned long _c = -read_csr(mcycle), _i = -read_csr(minstret);               \
        code;                                                                         \
        _c += read_csr(mcycle);                                                       \
        _i += read_csr(minstret);                                                     \
        if (cid == 0)                                                                 \
        {                                                                             \
            printf("Cycles: %lu, Instructions: %lu, Iterations: %d\n", _c, _i, iter); \
        }                                                                             \
    } while (0)

#endif // _UTIL_H