#ifndef _sm_regs_h
#define _sm_regs_h

#define SM_NPU_CORES 4
#define SM_HARTID_MCU0 0
#define SM_HARTID_NPU0 1
#define SM_HARTID_NPU1 2
#define SM_HARTID_NPU2 3
#define SM_HARTID_NPU3 4
#define SM_HARTID_NPUMAX 4

#define SM_DDR_BASE 0x80000000L  // 2GB
#define SM_DDR_SIZE 0x180000000L // 6GB
#define SM_DDR_END (SM_DDR_BASE + SM_DDR_SIZE - 1)

#define SM_L1_BASE 0x40000000                                    // 1GB
#define SM_L1_NPU_BASE(Core) (SM_L1_BASE + (Core * 0x20000000L)) // 512MB per NPU
#define SM_L1_NPU_SIZE 0x400000                                  // 4MB

#define SM_MCU_SRAM_BASE 0x3F000000 // 64MB
#define SM_MCU_SRAM_SIZE 0x800000   // 8MB

//---------------------------
// DISPATCH
//---------------------------
#define SM_DISPATCH_BASE 0x3C000000
#define SM_DISPATCH_START 0X0
#define SM_DISPATCH_STATUS 0X8
#define SM_DISPATCH_ECODE_BEGIN 0X10
#define SM_DISPATCH_ECODE_CORE(Core) (SM_DISPATCH_ECODE_BEGIN + (Core * 0x4))
#define SM_DISPATCH_ECODE_END SM_DISPATCH_ECODE_CORE(SM_NPU_CORES)
//EACH CORE 4 BYTES, MAX 32: 4*32
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
//ELSE BUSY
//BIT 0: 1: NPU0, 2: NPU1, 3: NPU2, 4: NPU3 BUSY

//---------------------------
//TCP
//---------------------------
#define SM_TCP_BASE 0x3D000000

//---------------------------
//FILE
//---------------------------
#define SM_FILE_BASE 0x3E000000

#endif // _sm_regs_h