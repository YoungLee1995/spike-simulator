/*addedd by li yang, date 2025-07-04
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "simif.h"
#include "sm_regs.h"
#include "sm_log.h"
#include "sm_file.h"

sm_file_t::sm_file_t(simif_t *simif)
{
    Simif = simif;
}

bool sm_file_t::load(reg_t addr, size_t len, uint8_t *bytes)
{
    U64 *p = (U64 *)bytes;
    if (addr == SM_FILE_OUT)
    {
        *p = out;
    }
}

bool sm_file_t::store(reg_t addr, size_t len, const uint8_t *bytes)
{
    U64 v = *(U64 *)bytes;
    if (addr == SM_FILE_CNTL)
    {
        do_op();
    }
    else if (SM_FILE_OP == addr)
    {
        op = v;
    }
    else if (SM_FILE_IN1 == addr)
    {
        in1 = v;
    }
    else if (SM_FILE_IN2 == addr)
    {
        in2 = v;
    }
    else if (SM_FILE_IN3 == addr)
    {
        in3 = v;
    }

    return true;
}

void sm_file_t::do_op()
{
    switch (op)
    {
    case sm_file_op_open:
    {
        U8 tmp;
        U32 idx = 0;
        // get the name
        do
        {
            Simif->mmio_load(in1 + idx, 1, &tmp);
            fbuff[idx] = tmp;
            idx++;
        } while (tmp);
        // open, mode_t needed when create a new file
        int flags = O_RDONLY;
        if (in2 & O_WRONLY)
            flags = O_CREAT | O_WRONLY | O_TRUNC;
        out = (U64)open(fbuff, flags, 0664);
        break;
    }

    case sm_file_op_close:
    {
        out = (U64)close((int)in1);
        break;
    }

    case sm_file_op_lseek:
    {
        out = (U64)lseek((int)in1, (off_t)in2, (int)in3);
        break;
    }

    case sm_file_op_read:
    {
        // in1: fd
        // in2: buffer
        // in3: count
        U64 off = 0;
        U64 left = in3;
        while (left > 0)
        {
            int siz = left > SM_FILE_BLEN ? SM_FILE_BLEN : left;
            Simif->mmio_load(in2 + off, siz, (U8 *)fbuff);
            siz = write(in1, (const void *)fbuff, siz);
            if (siz <= 0)
                break;
            left -= siz;
            off += siz;
        }
        // total read bits
        out = off;
        break;
    }

    case sm_file_op_write:
    {
        // in1: fd
        // in2: buffer
        // in3: count
        U64 off = 0;
        U64 left = in3;
        do
        {
            int siz = left > SM_FILE_BLEN ? SM_FILE_BLEN : left;
            int res = read(in1, (void *)fbuff, siz);
            Simif->mmio_store(in2 + off, res, (U8 *)fbuff);
            left -= res;
            off += res;
            if (res < siz)
                break;
        } while (left > 0);
        // total read bits
        out = off;
        break;
    }
    default:
        assert(0);
        break;
    }
}