.section .text
.globl _start
_start:
    li t0, 4
    vsetvli t0, t0, e32
    la a0, data1
    la a1, data2
    vle32.v v1, (a0)
    vle32.v v2, (a1)
    vadd.vv v3, v1, v2
    la a2, result
    vse32.v v3, (a2)
    ebreak

.section .data
data1:  .word 1, 2, 3, 4
data2:  .word 10, 20, 30, 40
result: .space 16
