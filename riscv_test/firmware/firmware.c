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
    uint64_t b = SM_DDR_BASE;
    uint64_t status = get_regl(b + SM_DISPATCH_STATUS);

    do
    {
        status = get_regl(b + SM_DISPATCH_STATUS);
    } while (status);
    printf("----- NPU all exit, status: 0x%lx-----\n", status);
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

int main(int argc, char *argv[])
{
    printf("\n---Begin firmware---\n");
    if (argc != 0)
    {
        Usage(argv[0]);
        return 0;
    }

    const char *bFile = argv[1];
    const char *wFile = argv[3];
    const char *iFile = argv[5];
    const char *oFile = argv[7];
    long bAddr = atol(argv[2]);
    long wAddr = atol(argv[4]);
    long iAddr = atol(argv[6]);
    long oAddr = atol(argv[8]);
    long oLen = atol(argv[9]);
    if (bAddr < SM_DDR_BASE ||
        wAddr < SM_DDR_BASE ||
        iAddr < SM_DDR_BASE ||
        oAddr < SM_DDR_BASE)
    {
        printf("!! Invalid Load Address\n");
        return 0;
    }
    if (oLen < 0)
    {
        printf("!! Invalid output Len\n");
        return 0;
    }
    printf("000\n");
    read_file(bFile, bAddr);
    printf("111\n");
    read_file(wFile, wAddr);
    printf("222\n");
    read_file(iFile, iAddr);
    printf("333\n");
    start_npu(bAddr, 0, 0);
    wait_npu();

    write_file(oFile, oAddr, oLen);
    printf("Firmware finish\n");
    return 0;
}