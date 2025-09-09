#include "../../riscv-isa-sim/smart/sm_regs.h"
#include "util.h"
#include <stdio.h>

// func2:       00
// sync_num:    4
// barrier_id:  2
// intra:       1
// func3:       010

#define CALL_BARRIER() ({                             \
    __asm__ __volatile__(".insn u 0xb,x0,0x08412\n\t" \
                         ".align 2 \n\t" ::           \
                             : "memory");             \
})

// func2:       00
// sync_num:    4
// barrier_id:  2
// intra:       1
// func3:       010
// main_flag:  1
void CALL_BARRIER_MAIN()
{
    __asm__ __volatile__(".word 0x0000000b |(%[rd]<<7) |(%[rs1]<<15) |(%[rs2]<<20) |(%[func3]<<12)|  (%[func7]<<25)\n\t"
                         :
                         : [rd] "i"(0x10), [rs1] "i"(2), [rs2] "i"(4), [func3] "i"(0x2), [func7] "i"(0x4)
                         : "memory");
}

void CALL_FENCE()
{
    // FENCE
    // imm> 31~20
    __asm__ __volatile__(".word 0x0000000f |(%[rd]<<7) |(%[rs1]<<15) |(%[imm]<<20) |(%[func3]<<12)\n\t"
                         :
                         : [rd] "i"(0x1c), [rs1] "i"(0), [func3] "i"(0), [imm] "i"(0x0)
                         : "memory");
}

void test_rvv(uint16_t *a, uint16_t *b, uint16_t *c)
{
    __asm__ __volatile__(".option norvc\n\t"
                         "vsetvli t0, %[vl], e16, m1\n\t"
                         "vlh.v v0, (%[a])\n\t"
                         "vlh.v v1, (%[b])\n\t"
                         "vadd.vv v2, v0, v1\n\t"
                         "vsh.v v2, (%[c])\n\t"
                         ".option rvc\n\t"
                         :
                         : [a] "r"(a), [b] "r"(b), [c] "r"(c), [vl] "r"(8)
                         : "t0", "v0", "v1", "v2", "memory");
}

#define saved_gp ((uint64_t *)(SM_MCU_SRAM_BASE + SM_MCU_SRAM_SIZE - 16))

int npu_proc(uint64_t arg0, uint64_t arg1)
{
    write_regl(gp, *saved_gp);
    write_csr(mstatus, (MSTATUS_FS | MSTATUS_XS | MSTATUS_VS));
    unsigned long hartid;
    hartid = read_csr(0xf14);

    printf("-------------test barrier and fence for main core=0-------------\n");
    printf("-- My hartid=%ld\n", hartid);
    printf("-- arg0:%lx,arg1:%lx\n", arg0, arg1);

    if(hartid == SM_HARTID_NPU0)
    {
        CALL_BARRIER_MAIN();
        CALL_FENCE();
        printf("hartid=%ld after main barrier\n", hartid);
    }
    else
    {
        CALL_BARRIER();
        printf("hartid=%ld after barrier\n", hartid);
        if(hartid == SM_HARTID_NPU1)
        {
            int16_t a[8] = {1, 2, 3, 4, 5, 6, 7, 8};
            int16_t b[8] = {10, 20, 30, 40, 50, 60, 70, 80};
            int16_t c[8] = {0};
            test_rvv(a, b, c);
            printf("hartid=%ld after rvv\n", hartid);
        }
    }
    
    uint64_t b = SM_DISPATCH_BASE;
    set_reg(b + SM_DISPATCH_ECODE_CORE(hartid - 1), 0);
    return 0;   
}


int start_npu()
{
    printf("-------start npu--------\n");
    uint64_t b = SM_DISPATCH_BASE;
    set_regl(b + SM_DISPATCH_PC, (uint64_t)npu_proc);
    set_regl(b + SM_DISPATCH_ARG0, 0x1234);
    set_regl(b + SM_DISPATCH_ARG1, 0xabcd);
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
    printf("----- NPU all exit, status: 0x%lx-----\n", status);
    return 0;
}

int main(int argc, char *argv[])
{
    printf("test barrier\n");
    register void *lgp asm("gp"); // Register the general-purpose register
    *saved_gp = (uint64_t)lgp;    // Save the current value of the general-purpose 
    
    start_npu();
    wait_npu();

    printf("test dispatch finish!\n");
    return 0; // Return success
}