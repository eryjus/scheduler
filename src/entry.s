;;===================================================================================================================
;;
;;  entry.s -- this is the entry point that multiboot is looking for
;;
;;  Handle a minimal setup and quickly hand control over to the kernel C code.  So, this setup will be:
;;  * set up a stack
;;  * create a GDT
;;  * create an IDT (which we will need later)
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  --------------------------------------------------------------------------
;;  2019-Sep-21  Initial   0.0.0   ADCL  Initial version
;;
;;===================================================================================================================


;;
;; -- things that need to be seen outside this source
;;    -----------------------------------------------
        global      entry


;;
;; -- things that need to be seen here but are outside this source
;;    ------------------------------------------------------------
        extern      kMain


;;
;; -- set up the multiboot header in its own section
;;    ----------------------------------------------
        section     .mboot

MAGIC   EQU         0x1badb002
FLAGS   EQU         0                   ;; -- no need for any info from MB

        align       4
mb_hdr:
        dd          MAGIC
        dd          FLAGS
        dd          -MAGIC-FLAGS

        align       16
gdtStart:
        dd          0x00000000,0x00000000       ;; this is the null segment descriptor
        dd          0x0000ffff,0x00cf9a00       ;; this is the kernel code segment
        dd          0x0000ffff,0x00cf9200       ;; this is the kernel stack (and data) segment
gdtEnd:

        align       8
gdtSize:
        dw          gdtEnd - gdtStart - 1
gdtLoc:
        dd          gdtStart

        align       8
idtSize:
        dw          0x7f
idtLoc:
        dd          0x17f000

;; 
;; -- This is the actual entry point
;;    ------------------------------
        section     .text
entry:
        mov         eax,idtSize
        lidt        [eax]

        mov         eax,gdtSize
        lgdt        [eax]

        ;;
        ;; -- create the stack at 0x180000 (1.5MB); 4K please
        ;;    -----------------------------------------------
        mov         eax,0x10
        mov         ds,ax
        mov         es,ax
        mov         fs,ax
        mov         gs,ax
        mov         ss,ax
        mov         esp,0x181000
        jmp         0x08:newGDT

newGDT:
        jmp         kMain

