// #include "../../riscv-isa-sim/smart/sm_regs.h"
// #include "fp16_fp32_func.h"
#include "util.h"
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/signal.h>
#include <stdarg.h>

#define SYS_write 64

#undef strcmp

extern volatile uint64_t tohost;
extern volatile uint64_t fromhost;
volatile uint32_t syscall_lock = 0;

static uintptr_t syscall(uintptr_t which, uint64_t arg0, uint64_t arg1, uint64_t arg2)
{
    volatile uint64_t magic_mem[8] __attribute__((aligned(64)));
    magic_mem[0] = which;
    magic_mem[1] = arg0;
    magic_mem[2] = arg1;
    magic_mem[3] = arg2;
    __sync_synchronize();
    while (__sync_lock_test_and_set(&syscall_lock, 1))
        ;
    tohost = (uintptr_t)magic_mem;
    while (fromhost == 0)
        ;
    fromhost = 0;

    __sync_synchronize();
    __sync_lock_release(&syscall_lock);
    return magic_mem[0];
}

#define NUM_COUNTERS 2
static uintptr_t counters[NUM_COUNTERS];
static const char *counter_names[NUM_COUNTERS];

void setStats(int enable)
{
    int i = 0;
#define READ_CTR(name)                  \
    do                                  \
    {                                   \
        while (i >= NUM_COUNTERS)       \
            ;                           \
        uintptr_t csr = read_csr(name); \
        if (!enable)                    \
        {                               \
            csr -= counters[i];         \
            counter_names[i] = #name;   \
        }                               \
        counters[i++] = csr;            \
    } while (0);

    READ_CTR(mcycle);
    READ_CTR(minstret);

#undef READ_CTR
}

void __attribute__((noreturn)) tohost_exit(uintptr_t code)
{
    tohost = (code << 1) | 1; // Set the exit code and signal completion
    while (1)
        ;
}

uintptr_t __attribute__((weak)) handle_trap(uintptr_t cause, uintptr_t epc, uint64_t regs[32])
{
    tohost_exit(1337);
    return 0;
}

void exit(int code) { tohost_exit(code); }

void abort(void)
{
    exit(128 + SIGABRT);
}

void printstr(const char *str)
{
    syscall(SYS_write, 1, (uintptr_t)str, strlen(str));
}

void __attribute__((weak)) thread_entry(int cid, int nc)
{
    while (cid != 0)
        ;
}

int __attribute__((weak)) main(int argc, char **argv)
{
    if (argc > 0 && argv[0] != NULL)
    {
        printstr("Implement main().\n");
        printstr(argv[0]);
    }
    else
        printstr("No program name provided.\n");

    return -1;
}

static void init_tls()
{
    register void *thread_pointer asm("tp");
    extern char _tdata_begin, _tdata_end, _tbss_begin, _tbss_end;
    size_t tdata_size = &_tdata_end - &_tdata_begin;
    memcpy(thread_pointer, &_tdata_begin, tdata_size);
    // size_t tbss_size = &_tbss_end - &_tbss_begin;
    size_t tbss_size = &_tbss_end - &_tdata_begin;
    memset(thread_pointer + tdata_size, 0, tbss_size);
}

void _init(int cid, int nc)
{
    init_tls();
    thread_entry(cid, nc);

    int ret = main(0, 0);

    char buf[NUM_COUNTERS * 32] __attribute__((aligned(64)));
    char *pbuf = buf;
    for (int i = 0; i < NUM_COUNTERS; i++)
    {
        if (counters[i] != 0)
        {
            pbuf += sprintf(pbuf, "%s: %lu\n", counter_names[i], counters[i]);
        }
    }
    if (pbuf != buf)
    {
        printstr(buf);
    }

    exit(ret);
}

#undef putchar
int putchar(int ch)
{
    static __thread char buf[64] __attribute__((aligned(64)));
    static __thread int buflen = 0;

    buf[buflen++] = ch;
    if (ch == '\n' || buflen >= sizeof(buf))
    {
        syscall(SYS_write, 1, (uintptr_t)buf, buflen);
        buflen = 0;
    }
    return 0;
}

void printhex(uint64_t value)
{
    char buf[16];
    char *p = buf + sizeof(buf) - 1;
    *p = '\0';
    do
    {
        int digit = value & 0xf;
        *(--p) = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        value >>= 4;
    } while (value != 0);

    printstr(p);
}

static inline void printnum(void (*punch)(int, void **), void **putdat,
                            unsigned long long value, unsigned base, int width, int pad)
{
    char buf[64];
    char *p = buf + sizeof(buf) - 1;
    *p = '\0';
    do
    {
        int digit = value % base;
        *(--p) = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
        value /= base;
    } while (value != 0);

    int len = buf + sizeof(buf) - 1 - p;
    if (len < width)
    {
        for (int i = 0; i < width - len; i++)
            *(--p) = pad ? '0' : ' ';
    }

    while (*p)
        punch(*p++, putdat);
}

static unsigned long long getuint(va_list *ap, int lflag)
{
    unsigned long long value;
    if (lflag & 1) // long
        value = va_arg(*ap, unsigned long);
    else if (lflag & 2) // long long
        value = va_arg(*ap, unsigned long long);
    else if (lflag & 4) // int
        value = va_arg(*ap, unsigned int);
    else // default is int
        value = va_arg(*ap, unsigned int);
    return value;
}

static void vprintfmt(void (*punch)(int, void **), void **putdat, const char *fmt, va_list ap)
{
    int lflag = 0; // length flag
    int width = 0; // field width
    int pad = 0;   // padding character
    while (*fmt)
    {
        if (*fmt == '%')
        {
            fmt++;
            if (*fmt == '0')
            {
                pad = 1;
                fmt++;
            }
            else
            {
                pad = 0;
            }

            if (*fmt >= '1' && *fmt <= '9')
            {
                width = 0;
                while (*fmt >= '0' && *fmt <= '9')
                    width = width * 10 + (*(fmt++) - '0');
            }
            else
            {
                width = 0;
            }

            switch (*fmt)
            {
            case 'd':
            case 'u':
                printnum(punch, putdat, getuint(&ap, lflag), 10, width, pad);
                break;
            case 'x':
                printnum(punch, putdat, getuint(&ap, lflag), 16, width, pad);
                break;
            case 'c':
                punch(va_arg(ap, int), putdat);
                break;
            case 's':
                printstr(va_arg(ap, char *));
                break;
            default:
                punch(*fmt, putdat);
                break;
            }
        }
        else
        {
            punch(*fmt++, putdat);
        }
    }
}

int printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    void *putdat = NULL; // No additional data needed for putchar
    vprintfmt((void *)putchar, 0, fmt, ap);
    va_end(ap);
    return 0; // Return value is not used in this implementation
}

int sprintf(char *str, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    char *str0 = str;

    void sprintf_punch(int ch, void **data)
    {
        char **pstr = (char **)data;
        **pstr =ch;
        (*pstr)++;
    }

    vprintfmt(sprintf_punch, (void**)&str, fmt, ap);
    *str=0;

    va_end(ap);
    return str -str0; // Return the number of characters written
}

void *memcpy(void *dest, const void *src, size_t len)
{
    if ((((uintptr_t)dest | (uintptr_t)src | len) & (sizeof(uintptr_t) - 1)) == 0)
    {
        // If the pointers and length are aligned, use word-sized copies
        uintptr_t *d = (uintptr_t *)dest;
        const uintptr_t *s = (const uintptr_t *)src;
        uintptr_t *end = dest + len;
        while (d + 8 < end)
        {
            uintptr_t reg[8] = {s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7]};
            d[0] = reg[0];
            d[1] = reg[1];
            d[2] = reg[2];
            d[3] = reg[3];
            d[4] = reg[4];
            d[5] = reg[5];
            d[6] = reg[6];
            d[7] = reg[7];
            d += 8;
            s += 8;
        }
        while (d < (uintptr_t *)end)
        {
            *d++ = *s++;
        }
    }
    else
    {
        // If not aligned, use byte-sized copies
        unsigned char *d = (unsigned char *)dest;
        const unsigned char *s = (const unsigned char *)src;
        while (d < (char *)(dest + len))
        {
            *d++ = *s++;
        }
    }
    return dest;
}

void *memset(void *dest, int byte, size_t len)
{
    if ((((uintptr_t)dest | len) & (sizeof(uintptr_t) - 1)) == 0)
    {
        // If the pointer and length are aligned, use word-sized sets
        uintptr_t word = (uintptr_t)byte & 0xff;
        word |= word << 8;
        word |= word << 16;
        word |= word << 32; // Extend to 64 bits if needed
        uintptr_t *d = (uintptr_t *)dest;
        while (d < (uintptr_t *)(dest + len))
        {
            *d++ = word;
        }
    }
    else
    {
        char *d = (char *)dest;
        while (d < (char *)(dest + len))
        {
            *d++ = (char)byte;
        }
    }
    return dest;
}

size_t strlen(const char *s)
{
    const char *p = s;
    while (*p)
        p++;
    return p - s;
}

int strcmp(const char *s1, const char *s2)
{
    unsigned char c1, c2;
    do
    {
        c1 = *s1++;
        c2 = *s2++;
    } while (c1 != 0 && c1 == c2);
    return c1 - c2;
}

char *strcpy(char *dest, const char *src)
{
    char *d = dest;
    while ((*d++ = *src++) != '\0')
        ;
    return dest;
}

long atol(const char *str)
{
    long result = 0;
    int sign = 1;

    // Skip whitespace
    while (*str == ' ' || *str == '\t')
        str++;

    // Handle sign
    if (*str == '-')
    {
        sign = -1;
        str++;
    }
    else if (*str == '+')
    {
        str++;
    }

    // Convert digits
    while (*str >= '0' && *str <= '9')
    {
        result = result * 10 + (*str - '0');
        str++;
    }

    return sign * result;
}