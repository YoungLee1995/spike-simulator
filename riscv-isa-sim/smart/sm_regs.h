#ifndef _sm_regs_h
#define _sm_regs_h

#define SM_NPU_CORES        4
#define SM_HARTID_MCU0      0
#define SM_HARTID_NPU0      1
#define SM_HARTID_NPU1      2
#define SM_HARTID_NPU2      3
#define SM_HARTID_NPU3      4
#define SM_HARTID_NPUMAX    4

// FPGA DEMO ADD
#define SM_DDR0_BASE        0x0UL
#define SM_DDR0_END         0x80000000UL
#define SM_DDR0_SIZE        (SM_DDR0_END - SM_DDR0_BASE) // 2GB
#define SM_DDR1_BASE        0x800000000UL
#define SM_DDR1_END         0xC00000000UL
#define SM_DDR1_SIZE        (SM_DDR1_END - SM_DDR1_BASE) // 16GB
#define SM_IS_DDR0(ADDR)    ((ADDR) >= SM_DDR0_BASE && (ADDR) <= SM_DDR0_END)
#define SM_IS_DDR1(ADDR)    ((ADDR) >= SM_DDR1_BASE && (ADDR) <= SM_DDR1_END)

#define SM_L1_BASE                  0x20100000000UL                                     
#define SM_L1_NPU_BASE(Core)        (SM_L1_BASE + ((Core) * 0x2000000UL))
#define SM_L1_NPU_SIZE              0x400000             
#define SM_L1_NPU_END(Core)         (SM_L1_NPU_BASE(Core) + SM_L1_NPU_SIZE)              
#define SM_NPU_END                  0x20140000000UL
#define SM_IS_L1BUF(Core, ADDR)     ((ADDR) >= SM_L1_NPU_BASE(Core) && (ADDR) <= SM_L1_NPU_END(Core))

#define SM_NPU_DBG_BASE 0XA4020000UL
#define SM_NPU_DBG_END  0XA4030000UL
#define SM_NPU_DBG_SIZE (SM_NPU_DBG_END-SM_NPU_DBG_BASE)

#define SM_MCU_SRAM_BASE        0x7F000000UL // 64MB
#define SM_MCU_SRAM_SIZE        0x800000     // 8MB

//---------------------------
// DISPATCH
//---------------------------
#define SM_BARRIER_COUNT 32
//---------------------------
// DISPATCH
//---------------------------
#define SM_DISPATCH_BASE 0x7C000000UL
#define SM_DISPATCH_START 0X0
#define SM_DISPATCH_STATUS 0X8
#define SM_DISPATCH_ECODE_BEGIN 0X10
#define SM_DISPATCH_ECODE_CORE(Core) (SM_DISPATCH_ECODE_BEGIN + ((Core) * 0x4))
#define SM_DISPATCH_ECODE_END SM_DISPATCH_ECODE_CORE(SM_NPU_CORES)
// EACH CORE 4 BYTES, MAX 32: 4*32
#define SM_DISPATCH_PC 0XA0
#define SM_DISPATCH_ARG0 0XA8
#define SM_DISPATCH_ARG1 0XB0
#define SM_DISPATCH_ARG2 0XB8
#define SM_DISPATCH_ARG3 0xC0
#define SM_DISPATCH_ARG4 0xC8
#define SM_DISPATCH_ARG5 0xD0
#define SM_DISPATCH_ARG6 0xD8
#define SM_DISPATCH_ARG7 0xE0

#define SM_DISPATCH_STATUS_IDLE 0x0
// ELSE BUSY
// BIT 0: 1: NPU0, 2: NPU1, 3: NPU2, 4: NPU3 BUSY

//---------------------------
// TCP
//---------------------------
#define SM_TCP_BASE 0x7D000000UL

//---------------------------
// FILE
//---------------------------
#define SM_FILE_BASE 0x7E000000UL
// Model file interface
#define SM_FILE_CNTL 0X0
#define SM_FILE_OP 0X8
#define SM_FILE_OUT 0X10
#define SM_FILE_IN1 0X18
#define SM_FILE_IN2 0X20
#define SM_FILE_IN3 0X28
typedef enum
{
    sm_file_op_open,
    sm_file_op_close,
    sm_file_op_lseek,
    sm_file_op_read,
    sm_file_op_write
} sm_file_op_t;

#endif // _sm_regs_h