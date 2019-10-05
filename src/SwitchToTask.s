;;===================================================================================================================
;;
;;  SwitchToTask.s -- This will perform the actual task switch
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  --------------------------------------------------------------------------
;;  2019-Sep-22  Initial   step01   ADCL  Initial version
;;  2019-Sep-26  Step 01   step01   ADCL  Fix a stack symmetry issue
;;
;;===================================================================================================================


;;
;; -- things that need to be seen outside this source
;;    -----------------------------------------------
        global      SwitchToTask        ;; void SwitchToTask(PCB_t *task) __attributes__((cdecl));
        global      GetCR3              ;; unsigned int GetCR3(void);


;;
;; -- thing that need to be see inside this source but are defined outside
;;    --------------------------------------------------------------------
        extern      currentPCB
        extern      UpdateTimeUsed


;;
;; -- Some offsets into the PCB sructure
;;    ----------------------------------
TOS     equ         0
VAS     equ         4


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
        section     .text
        cpu         586


;;
;; -- Perform the actual task switch
;;   
;;    Some notes about this function:
;;    * cdecl ensures that `eax`, `ecx`, and `edx` are saved on the stack
;;    * `eip` was also saved by the `call` instruction
;;    * `cr3` does not need to be saved -- once set it cannot be changed
;;    * segment registers do not need to be saves -- kernel segments to kernel segments
;;    
;;    IRQs are required to be disabled prior to calling and restored on exit
;;    ---------------------------------------------------------------------------------
SwitchToTask:
        push        ebx
        push        esi
        push        edi
        push        ebp

        call        UpdateTimeUsed          ;; update the time used field before swapping

        mov         edi,[currentPCB]        ;; `edi` = previous tasks PCB
        mov         [edi+TOS],esp           ;; save the top of the stack


        ;; -- load the next task's state


        mov         esi,[esp+((4+1)*4)]     ;; `esi` = next tasks PCB
        mov         [currentPCB],esi        ;; save the current process PCB

        mov         esp,[esi+TOS]           ;; load the next process's stack
        mov         eax,[esi+VAS]           ;; load the next process's virtual address space

        ;; -- fix tss.esp0 here

        mov         ecx,cr3                 ;; get the current task cr3

        cmp         ecx,eax                 ;; are the virtual address spaces the same
        jmp         .sameVAS                
        mov         cr3,eax                 ;; update to the new virtual address space

.sameVAS:
        pop         ebp
        pop         edi
        pop         esi
        pop         ebx

        ret                                 ;; this is the next task's `eip`


;;
;; -- get the current `cr3` value
;;    ---------------------------
GetCR3:
        mov         eax,cr3
        ret
