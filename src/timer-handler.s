;;===================================================================================================================
;;
;;  timer-handler.s -- This is the timer ISR routine, which will handle keeping the counter up to date
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  --------------------------------------------------------------------------
;;  2019-Sep-29  Initial   step03  ADCL  Initial version
;;
;;===================================================================================================================


;;
;; -- things that need to be seen outside this source
;;    -----------------------------------------------
        global      TimerHandler


;;
;; -- things that need to be seen here but are outside this source
;;    ------------------------------------------------------------
        extern      counter


;; 
;; -- This is the actual entry point
;;    ------------------------------
        section     .text
TimerHandler:
        inc         dword [counter]

;;
;; -- issue an EOI
;;    ------------
        push        eax
        mov         eax,0x20
        out         0x20,al
        pop         eax

        iret

