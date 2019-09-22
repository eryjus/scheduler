#####################################################################################################################
##
##  Makefile -- This is the core makefile for the scheduler test
##
##        Copyright (c)  2019 -- Adam Clark; See LICENSE.md
##
## -----------------------------------------------------------------------------------------------------------------
##
##     Date      Tracker  Version  Pgmr  Description
##  -----------  -------  -------  ----  ---------------------------------------------------------------------------
##  2019-Sep-21  Initial   0.0.0   ADCL  Initial version
##
#####################################################################################################################


.SILENT:

X86-LIB = $(shell i686-elf-gcc --print-libgcc-file-name)


##
## -- This is the default rule, to compile everything
##    -----------------------------------------------
.PHONY: all
all: init
	tup


##
## -- This rule will make sure that up is initialized and that we have created all the proper variants
##    ------------------------------------------------------------------------------------------------
.PHONY: init
init: tuprules.inc
	if [ ! -d .tup ]; then tup init; fi;


##
## -- we need to know the current base folder
##    ---------------------------------------
tuprules.inc: Makefile
	echo WS = `pwd` > $@
	echo X86_LDFLAGS = $(dir $(X86-LIB)) >> $@


##
## -- This is the rule to build the x86-pc bootable image
##    ---------------------------------------------------
.PHONY: x86-pc
x86-pc: init all
	rm -fR img/x86-pc.iso
	rm -fR sysroot/*
	mkdir -p sysroot img
	cp -fR bin/* sysroot/
	find sysroot -type f -name Tupfile -delete
	grub2-mkrescue -o img/x86-pc.iso sysroot


##
## -- Run the x86-pc on qemu
##    ----------------------
.PHONY: run-x86-pc
run-x86-pc: x86-pc
	qemu-system-i386 -smp 4 -m 3584 -serial stdio -cdrom img/x86-pc.iso


