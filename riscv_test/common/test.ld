/* Proxy kernel link script */
/*this is the linker script used when building the Proxy kernel*/
/*the output_arch command specifies the machine architecture  where the 
argument is one of the names used in the BFD library. More specifically
on of the entires in bfd/cpu-mips.c*/

OUTPUT_ARCH("riscv")
ENTRY(_start)

SECTIONS
{
    /* text: test code section */
    . = 0x3f000000;
    .text.init : { *(.text.init)}

    . = ALIGN(0x1000);
    .tohost : { *(.tohost)}

    . = ALIGN(0x1000);
    .text : { *(.text)}

    /*data segment*/
    .data : {*(.data)}

    .sdata : {
        __global_pointer$ = . + 0x800;
        *(.srodata.cst16) *(.srodata.cst8) *(.srodata.cst4) *(.srodata.cst2) *(.srodata)
        *(.sdata .sdata* .gnu.linkonce.s.*)
    }

    /* bss segment*/
    .sbss : {
        *(.sbss .sbss.* .gnu.linkonce.sb.*)
        *(.scommon)
    }
    .bss : {*(.bss)}

    /* thread-local data segment*/
    .tdata :
    {
        _tdata_begin = .;
        *(.tdata)
        _tdata_end = .;
    }
    .tbss :
    {
        *(.tbss)
        _tbss_end = .;
    }

    /* End of unintialized data segment*/
    _end = .;
}