

__asm__ __volatile__(
    "option norvc \n\t"
    "li t0, 0x00000007 \n\t"
    "li t1, 16\n\t"
    "vsetvli t0,x0,e32,m1,ta,ma\n\t"
    "add t0, %[data_ptr],64\n\t"
    "vle32.v v0, (%[data_ptr])\n\t"
    "vle32.v v1, (t0)\n\t"
    ".word 0x0000000b| (%[rd]<<7) | (%[rs1]<<15) | (%[rs2]<<20) | (%[func3]<<12)| (%[func7]<<25)\n\t"
    :
    :[data_ptr] "r"(vector_data), [rd] "i"(0), [rs1] "i"(0), [rs2] "i"(0), [func3] "i"(i), [func7] "i"(0x40)
    :"t0", "memory");