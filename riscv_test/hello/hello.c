#include <stdio.h>
#include "util.h"

int main() {
    printf("Hello, RISC-V!\n");
    unsigned long hartid ;
    hartid = read_csr(0xF14); // Read the hart ID from the CSR
    printf("Hart ID: %ld\n", hartid);
    printf("This is a RISC-V test program.\n");
    return 0;
}