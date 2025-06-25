.section .text.init
.globl _start
_start:
  # 清寄存器（略去）

  # 设置 mstatus 启用向量
  li t0, 0x6000
  csrs mstatus, t0

  # 设置 mtvec（可选）
  la t0, trap_entry
  csrw mtvec, t0

  # 设置栈
  la sp, _stack_top

  # 跳转到用户定义入口
  j _init

trap_entry:
  ebreak

.section .bss
.space 4096
.globl _stack_top
_stack_top:
