#include "util.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "../../riscv-isa-sim/smart/sm_regs.h"

off_t get_fsize(const char *fname)
{
    int fd = open(fname, O_RDONLY);
    if (fd < 0)
    {
        printf("!! %s don't exist.\n", fname);
        exit(0);
    }

    off_t fsize = lseek(fd, 0, SEEK_END);
    if (fsize <= 0)
    {
        printf("!! %s is empty.\n", fname);
        exit(0);
    }

    close(fd);
    return fsize;
}

int read_file(const char *fname, u_int64_t addr)
{
    int fd;
    off_t fsize;
    fsize = get_fsize(fname);
    printf("------read: %s, Size:%lx, Addr:0x%lx\n", fname, fsize, addr);

    fd = open(fname, O_RDONLY);
    read(fd, (void *)addr, fsize);
    close(fd);
    return 0;
}

int write_file(const char *fname, u_int64_t addr, int len)
{
    int fd;
    fd = open(fname, O_RDONLY | O_WRONLY | O_TRUNC);
    if (fd < 0)
    {
        printf("!!create file %s failed, don't exist.\n", fname);
        exit(0);
    }
    printf("------write: %s, Size:%lx, Addr:0x%lx\n", fname, len, addr);
    write(fd, (void *)addr, len);
    close(fd);
    return 0;
}

int start_npu(uint64_t pc, uint64_t arg0, uint64_t arg1)
{
    printf("-------start npu--------\n");
    uint64_t b = SM_DISPATCH_BASE;
    set_regl(b + SM_DISPATCH_PC, pc);
    set_regl(b + SM_DISPATCH_ARG0, arg0);
    set_regl(b + SM_DISPATCH_ARG1, arg1);
    set_regl(b + SM_DISPATCH_START, 1);
    return 0;
}

int wait_npu()
{
    printf("------wait npu---------\n");
    uint64_t b = SM_DISPATCH_BASE;
    uint64_t status = get_regl(b + SM_DISPATCH_STATUS);

    do
    {
        status = get_regl(b + SM_DISPATCH_STATUS);
    } while (status);
    return 0;
}

void Usage(const char *fname)
{
    printf("\n");
    printf("usage: spike %s <Binary> <Addr> <Weight> <Addr> <Input> <Addr> <Output> <Addr> \n", fname);
    printf("Parameters:\n");
    printf("    Binary: Executable File (Binary format)\n");
    printf("    Weight: Weight File\n");
    printf("    Input:  Input File\n");
    printf("    Output: Output File\n");
    printf("\n");
}

void UsageEnh(const char *fname)
{
    // for matmul + elw + pproc
    printf("\n");
    printf("Another usage: spike %s <Binary> <Addr> <Weight> <Addr> <Input> <Addr> "
           " <Element> <Addr> <Scale> <Addr>"
           "<Output> <Addr> \n",
           fname);
    printf("Parameters:\n");
    printf("    Binary: Executable File (Binary format)\n");
    printf("    Weight: Weight File\n");
    printf("    Input:  Input File\n");
    printf("    Element: Element-wise File\n");
    printf("    Scale: Scale File\n");
    printf("    Output: Output File\n");
    printf("\n");
}

int main(int argc, char *argv[])
{
    printf("\n---Begin firmware---\n");
    if (argc < 10) // argv size 10 or 14
    {
        Usage(argv[0]);
        UsageEnh(argv[0]);
        return 0;
    }

    const char *bFile = argv[1];
    const char *wFile = argv[3];
    const char *iFile = argv[5];
    const char *eFile = (argc == 14) ? argv[7] : argv[5];
    const char *sFile = (argc == 14) ? argv[9] : argv[3];
    const char *oFile = (argc == 14) ? argv[11] : argv[7];
    long bAddr = atol(argv[2]);
    long wAddr = atol(argv[4]);
    long iAddr = atol(argv[6]);
    long eAddr = (argc == 14) ? atol(argv[8]) : atol(argv[6]);
    long sAddr = (argc == 14) ? atol(argv[10]) : atol(argv[4]);
    long oAddr = (argc == 14) ? atol(argv[12]) : atol(argv[8]);
    long oLen = (argc == 14) ? atol(argv[13]) : atol(argv[9]);
    if (bAddr > SM_DDR1_END ||
        wAddr > SM_DDR1_END ||
        iAddr > SM_DDR1_END ||
        oAddr > SM_DDR1_END ||
        eAddr > SM_DDR1_END ||
        sAddr > SM_DDR1_END)
    {
        printf("!! Invalid Load Address\n");
        return 0;
    }
    if (oLen < 0)
    {
        printf("!! Invalid output Len\n");
        return 0;
    }
    read_file(bFile, bAddr);
    read_file(wFile, wAddr);
    read_file(iFile, iAddr);
    read_file(eFile, eAddr);
    read_file(sFile, sAddr);
    start_npu(bAddr, 0, 0);
    wait_npu();

    write_file(oFile, oAddr, oLen);

    int Orescnt = 0;
    const char *fName;
    long fAddr = SM_DDR1_BASE, flen = 0;
    if (argc > 10 && (argc - 10) % 3 == 0)
    {
        Orescnt = (argc - 10) / 3;
        for (int i = 0; i < Orescnt; i++)
        {
            fName = argv[10 + i * 3];
            fAddr = atol(argv[11 + i * 3]);
            flen = atol(argv[12 + i * 3]);
            write_file(fName, fAddr, flen);
        }
    }
    if (argc > 14 && (argc - 14) % 3 == 0)
    {
        Orescnt = (argc - 14) / 3;
        for (int i = 0; i < Orescnt; i++)
        {
            fName = argv[14 + i * 3];
            fAddr = atol(argv[15 + i * 3]);
            flen = atol(argv[16 + i * 3]);
            write_file(fName, fAddr, flen);
        }
    }
    printf("Firmware finish\n");
    return 0;
}