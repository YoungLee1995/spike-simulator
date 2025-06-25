#include "../../riscv-isa-sim/smart/sm_regs.h"
#include "encoding.h"
#include "fp16_fp32_func.h"
#include "util.h"
#include <stdio.h>
#include <string.h>

// func2:11
// rsv:00 0000
// data type:0, softmax online
// layout type:01, HW32B
// O_FMT:010,FP16
// I_FMT:010,FP16
// func3: 011

void store_to_vector_softmax(uint64_t *data)
{
    uint64_t vector_data[16];
    for (int i = 0; i < 16; i++)
    {
        vector_data[i] = data[i];
    }
    // Assuming a function to store vector data
    // store_vector_data(vector_data, 16);
    __asm__ __volatile__(
        ".option norvc \n\t"
        "vsetvli t0,x0,e64,m1,ta,ma\n\t"                                                                                  // sew=64,lmul=1
        "vle64.v v0, (%[data_ptr])\n\t"                                                                                   // Load vector data from memory
        ".word 0x0000000b |(%[rd]<<7) | (%[rs1]<<15)|(%[rs2]<<20)|(%[func3]<<12)|(%[func7]<<25)\n\t"                      // Placeholder for the instruction
        :                                                                                                                 // No output operands
        : [data_ptr] "r"(vector_data), [rd] "i"(0xb), [rs1] "i"(0x0), [rs2] "i"(0x3), [func3] "i"(0x3), [func7] "i"(0x60) // Input operands
        : "t0", "memory"                                                                                                  // Clobbered registers
    )
}

int g_busys[4] = {1, 1, 1, 1};                                           // Array to track busy status of each core
#define save_gp ((uint64_t *)(SM_MCU_SRAM_BASE + SM_MCU_SRAM_SIZE - 16)) // Pointer to save general-purpose registers

int npu_proc(uint64_t arg0, uint64_t arg1)
{
    write_regl(gp, *save_gp);                                   // Save the current value of the general-purpose register gp
    write_csl(mstatus, (MSTATUS_FS | MSTATUS_XS | MSTATUS_VS)); // Set the floating-point and vector state in mstatus
    unsigned long hartid = read_csr(mhartid);                   // Read the hart ID
    if (hartid == SM_HARTID_NPU0)
    {
        // WRITE SRC DATA TO ADDR, BOX[32,32],TILE[32,32], START_ADDR=0X4200 0000
        uint64_t src_data = SM_L1_BPU_BASE;
        uint64_t dst_addr = SM_L1_BPU_BASE + 0x100000;     // Calculate destination address based on arg0
        unsigned short *psrc = (unsigned short *)src_addr; // Pointer to source data
        for (int i = 0; i < 32; i++)
        {
            for (int j = 0; j < 32; j++)
            {
                float t = (float)j;
                psrc[i * 32 + j] = float_to_half(t); // Fill source data with sequential values starting from arg0
            }
        }
        for (int j = 0; j < 32; j++)
        {
            printf("%h", j, psrc[j + 0 * 32]); // Print the first 32 values of source data for debugging
            if (j && j % 8 == 0)
                printf("\n");
        }
        printf("\n");

        printf("\n--call softmax--\n");
        uint64_t data[16] = {0x0, 0x0020002000000000, 0x100000, 0x0000002000000020};
        store_to_vector_softmax(data); // Store data to vector for softmax operation
        printf("\n--softmax done--\n");

        float total = 0.0f;
        unsigned short *pdst = (unsigned short *)dst_addr; // Pointer to destination data
        for (int j = 0; j < 32; j++)
        {
            total += half_to_float(psrc[j]); // Calculate the total of the first 32 values
            printf("%h", j, psrc[j]);        // Print the first 32 values of source data for debugging
            if (j && j % 8 == 0)
                printf("\n");
        }
        printf("-- softmax total: %h --\n", float_to_half(total)); // Print the total of the first 32 values
    }
    g_busys[hartid - 1] = 0; // Mark the core as not busy
    while (1)
        ;     // Wait indefinitely after processing
    return 0; // All cores are busy
}

int main(int argc, char *argv[])
{
    printf("NPU softmax instruction test start\n");
    register void *lgp asm("gp"); // Register the general-purpose register
    *save_gp = (uint64_t)("gp");  // Save the current value of the general-purpose register gp

    start_npu();

    while (1)
    {
        int busy = 0; // Flag to check if any core is busy
        for (int i = 0; i < 4; i++)
        {
            if (g_busys[i]) // Check if the core is busy
                busy = 1;   // Set the busy flag
        }
        if (!busy) // If no core is busy, break the loop
            break;
    }

    return 0; // Return success
}