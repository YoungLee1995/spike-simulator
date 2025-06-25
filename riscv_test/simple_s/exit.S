.section .text
.globl _start
_start:
    li a0, 10     # exit code
    li a7, 93     # syscall number for exit
    ecall         # make syscall
