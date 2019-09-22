/*******************************************************************************************************************/
/*                                                                                                                 */
/*  kernel.ld -- This is the linker script for locating the sections in the target binary                          */
/*                                                                                                                 */
/*        Copyright (c)  2019 -- Adam Clark                                                                        */
/*                                                                                                                 */
/* --------------------------------------------------------------------------------------------------------------- */
/*                                                                                                                 */
/*     Date      Tracker  Version  Pgmr  Description                                                               */
/*  -----------  -------  -------  ----  ------------------------------------------------------------------------  */
/*  2019-Sep-14  Initial   0.0.0   ADCL  Initial version                                                           */
/*                                                                                                                 */
/*******************************************************************************************************************/

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(entry)

LOCN = 0x00100000;                      /* We will need this value for the loader */

SECTIONS {
    . = LOCN;                           /* Multiboot will place the kernel here */


    _loaderStart = .;
    .text : {       /* This is where the loader will be located -- things to be reclaimed: */
        *(.mboot)                       /* -- multiboot header comes first */
        *(.init)
        *(.text .text.* .stub .gnu.linkonce.t.*)
    }

    .rodata : {
        *(.rodata .rodata.* .gnu.linkonce.r.*)
    }

    .stab : {
        *(.stab);
        *(.stabstr);
        *(.debug*);
        *(.got*)
        BYTE(0)        /* Force the linker to allocate space for this section */
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }

    /DISCARD/ : {
        *(.eh_frame .note.GNU-stack .comment .bootstack)
    }
}
