# Scheduler Test Journal

As always, I am going to produce a journal of my efforts with a particular project.  In this case, the project is to work out the kinks with some purpose-built code for the scheduler.  

Yesterday, I spent the time to get a boot image built and able to boot and print a *welcome* message.  It is the most trivial "Hello World" application that can be built with a multiboot bootloader.

Be sure to see [README.md](README.md) and [LICENSE.md](LICENSE.md) for more information.


### 2019-Sep-22

So, since I am working from Brandan's tutorial in an isolated environment, I will be working as closely to his tutorial as I can.  I will likely pull small bits of code from Century-OS, but just to save me some typing.  I will strip the code as necessary to match Brendan's tutorial.

Part 1 is to write a function called `Switch_to_task()`.  In CenturyOS, this function was `ProcessSwitch()`.  It was written in assembly language as it should be -- it is architecture dependent.  Also started in this step is the data structure needed to keep track of what a "Process" is.

I wanat to call out here the Brendan is assuming that paging is enabled.  In my tests, it is not.  Therefore, `cr3` is not a relevant register to change.  I will write the code to update it, but in reality if I write my code properly it will never have to change (all `0` values).  If for some reason it does change, the `PG` flag is not set so there should be no ramifications from the change -- there should be not `#PF` or `#GP` to worry about.

---

## Branch `step01`

I want to start with the `ProcessControlBlock` structure.

When I get to the `SwitchToTask()` function, there are structure elements for the TSS (which will be used for transitions between privilege levels) in the tutorial but without any further commentary.  Since I am at the moment completely at ring 0 and dont even have a TSS built.  For the moment, I am going to bypass this particular detail and add it back in later when I get to User Space.

---

The next thing to do is to initialize the scheduler with the running process.  

OK, so the tutorial wants to allocate some memory for a new `PCB_t` type.  The term "allocate" could also be from an statically allocated array.  This will get me around having to have a heap to allocate memory from.  So, I will use an array -- but that also means that I will need to initialize this array (remember that I am not initializaing my `.bss` data to 0).  I will add an allocated field to the `PCB_t` struture and initialize that to 0 in `InitSheduler()`.

This quick change will now allow me to create `PcbAlloc()` and `PcbFree()` functions to replace the similar heap-related functions.

It's time to talk a little about stack allocations.  The stacks are intended to be allocated from `0x180000` on up.  Each process will get it's own 4K (`0x1000`) long stack.  So the stack can be calculated easy enough based on the index of `pcbarray` that is allocated.  The following formula can be used: `((0x180 * i) << 12) + 0x1000` will provide the proper top of stack.  This can be done in `PcbAlloc()`.

I will also be creating a startup function (`ProcessStartup()`) which will also need to be added to the stack.

---

OK, so I finally have something that is complete and compiles.  However, I am triple faulting.  So, now it is time to get into debugging mode.  Right now, the best thing is bochs.  Bochs says:

```
00018109499i[BIOS  ] Booting from 07c0:0000
00143437569e[CPU0  ] interrupt(): gate descriptor is not valid sys seg (vector=0x06)
00143437569e[CPU0  ] interrupt(): gate descriptor is not valid sys seg (vector=0x0d)
00143437569e[CPU0  ] interrupt(): gate descriptor is not valid sys seg (vector=0x08)
00143437569i[CPU0  ] CPU is in protected mode (active)
00143437569i[CPU0  ] CS.mode = 32 bit
00143437569i[CPU0  ] SS.mode = 32 bit
00143437569i[CPU0  ] EFER   = 0x00000000
00143437569i[CPU0  ] | EAX=00000000  EBX=00010000  ECX=00102ad0  EDX=000b8000
00143437569i[CPU0  ] | ESP=00180ff8  EBP=00000000  ESI=00000000  EDI=001000a0
00143437569i[CPU0  ] | IOPL=0 id vip vif ac vm RF nt of df if tf SF zf af pf cf
00143437569i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
00143437569i[CPU0  ] |  CS:0008( 0001| 0|  0) 00000000 ffffffff 1 1
00143437569i[CPU0  ] |  DS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00143437569i[CPU0  ] |  SS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00143437569i[CPU0  ] |  ES:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00143437569i[CPU0  ] |  FS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00143437569i[CPU0  ] |  GS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00143437569i[CPU0  ] | EIP=00000003 (00000003)
00143437569i[CPU0  ] | CR0=0x60000011 CR2=0x00000000
00143437569i[CPU0  ] | CR3=0x00102ad0 CR4=0x00000000
(0).[143437569] [0x000000000003] 0008:0000000000000003 (unk. ctxt): lock push ebx             ; f053
00143437569p[CPU0  ] >>PANIC<< exception(): 3rd (13) exception with no resolution
```

So, the first thing to work out is the exception 0x06.  This is an `#UD`.  This means I am trynig to execute some garbage.  `eip` is `0x00000003` which makes a ton of sense -- I am jumping to `0x00000000`.

So, this should be from the stack for a new process.  I should be able to confirm this by commenting out the call to `SwitchToTask(B);`  So, this is confirmed.

Turns out my stack was upside-down.  So, I fixed that and I only get 1 task swap per from each.

Bochs shows the following:

```
00018109499i[BIOS  ] Booting from 07c0:0000
00143437624e[CPU0  ] interrupt(): gate descriptor is not valid sys seg (vector=0x06)
00143437624e[CPU0  ] interrupt(): gate descriptor is not valid sys seg (vector=0x0d)
00143437624e[CPU0  ] interrupt(): gate descriptor is not valid sys seg (vector=0x08)
00143437624i[CPU0  ] CPU is in protected mode (active)
00143437624i[CPU0  ] CS.mode = 32 bit
00143437624i[CPU0  ] SS.mode = 32 bit
00143437624i[CPU0  ] EFER   = 0x00000000
00143437624i[CPU0  ] | EAX=00102ad0  EBX=00010000  ECX=00102ad0  EDX=000b8000
00143437624i[CPU0  ] | ESP=00000014  EBP=f000ff53  ESI=f000ff53  EDI=f000ff53
00143437624i[CPU0  ] | IOPL=0 id vip vif ac vm RF nt of df if tf sf ZF af PF cf
00143437624i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
00143437624i[CPU0  ] |  CS:0008( 0001| 0|  0) 00000000 ffffffff 1 1
00143437624i[CPU0  ] |  DS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00143437624i[CPU0  ] |  SS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00143437624i[CPU0  ] |  ES:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00143437624i[CPU0  ] |  FS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00143437624i[CPU0  ] |  GS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00143437624i[CPU0  ] | EIP=f000ff53 (f000ff53)
00143437624i[CPU0  ] | CR0=0x60000011 CR2=0x00000000
00143437624i[CPU0  ] | CR3=0x00102ad0 CR4=0x00000000
bx_dbg_read_linear: physical memory read error (phy=0x0000f000ff53, lin=0x00000000f000ff53)
00143437624p[CPU0  ] >>PANIC<< exception(): 3rd (13) exception with no resolution
```

So, now, I believe I am having a problem with either the initialiation or the task swap code saving the state for the initial task.

### -- 2019-Sep-23

More debugging....  I pick up with the second `SwitchToTask()` call which never completes properly....

---

Well, crap!

```asm
;
;; -- get the current `cr3` value
;;    ---------------------------
GetCR3:
        mov         cr3,eax
        ret
```

... that `mov` is in the wrong order.  I'm outta practice!

Too bad it's not my only error.

---

### 2019-Sep-24

Well, I managed upon another stupid mistake:

```asm
TOS     equ         0
VAS     equ         4

;; <snip>

        mov         [edi+ESP],esp           ;; save the top of the stack
```

I renamed ESP to be TOS due to my stupidity and I missed one.

And, *finally* that appears to work.

---

After completing a review and updating some things, I ran another test with Bochs to make sure I did not break anything.  I did (or rather never got it right to begin with).

```
00018109499i[BIOS  ] Booting from 07c0:0000
00151549639e[CPU0  ] interrupt(): gate descriptor is not valid sys seg (vector=0x06)
00151549639e[CPU0  ] interrupt(): gate descriptor is not valid sys seg (vector=0x0d)
00151549639e[CPU0  ] interrupt(): gate descriptor is not valid sys seg (vector=0x08)
00151549639i[CPU0  ] CPU is in protected mode (active)
00151549639i[CPU0  ] CS.mode = 32 bit
00151549639i[CPU0  ] SS.mode = 32 bit
00151549639i[CPU0  ] EFER   = 0x00000000
00151549639i[CPU0  ] | EAX=f000ffd3  EBX=00010001  ECX=00000000  EDX=000b8000
00151549639i[CPU0  ] | ESP=f000ff63  EBP=ffffffff  ESI=ffffffff  EDI=ffffffff
00151549639i[CPU0  ] | IOPL=0 id vip vif ac vm RF nt of df if tf sf ZF AF PF cf
00151549639i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
00151549639i[CPU0  ] |  CS:0008( 0001| 0|  0) 00000000 ffffffff 1 1
00151549639i[CPU0  ] |  DS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00151549639i[CPU0  ] |  SS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00151549639i[CPU0  ] |  ES:00f6( 001e| 1|  2) f053f000 0000ff53 0 0
00151549639i[CPU0  ] |  FS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00151549639i[CPU0  ] |  GS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00151549639i[CPU0  ] | EIP=00000007 (00000007)
00151549639i[CPU0  ] | CR0=0x60000011 CR2=0x00000000
00151549639i[CPU0  ] | CR3=0x00000000 CR4=0x00000000
(0).[151549639] [0x000000000007] 0008:0000000000000007 (unk. ctxt): lock push ebx             ; f053
```

So, the screen fills properly with "ABC" and only after that does the code walk off to nowhere.  My expectation is that the code will continue to execute forever and not triple fault.

So, I still have a problem to solve.

Well, the `pcbArray` looks proper:

```
<bochs:5> xp /24 0x101800
[bochs]:
0x0000000000101800 <bogus+       0>:    0x00180fd0      0x00000000      0x00000000      0x00000000
0x0000000000101810 <bogus+      16>:    0x00000001      0x00181fcc      0x00000000      0x00000000
0x0000000000101820 <bogus+      32>:    0x00000000      0x00000001      0x00182fd8      0x00000000
0x0000000000101830 <bogus+      48>:    0x00000000      0x00000000      0x00000001      0x00000000
0x0000000000101840 <bogus+      64>:    0x00000000      0x00000000      0x00000000      0x00000000
0x0000000000101850 <bogus+      80>:    0x00000000      0x00000000      0x00000000      0x00000000
```

The stacks are not in conflict.

And, here is the problem:

This was from the execution log:

```asm
IN: 
0x00100040:  43                       incl     %ebx
0x00100041:  07                       popl     %es
```

And this is from the disassembled code:

```asm
00100040 <SwitchToTask>:
  100040:       53                      push   %ebx
  100041:       56                      push   %esi
```

Well, I am overwriting the code with the video buffer.  This is supposed to not happen but it is.

---

### 2019-Sep-25

## Branch `step02`

Step 2 is a near trivial implementation of a `Schedule()` function and then some resulting cleanup that this new function enables.  This will entail:
* Creating and maintaing a circular linked list, updating `InitScheduler()` and `CreateProcess()` -- and I want to note that I do then expect the processes to execute in reverse order from which they were created
* Write a `Schedule()` function
* Replace calls to `SwitchToTask()` with a call to `Schedule()` instead -- which will make `ProcessB()` and `ProcessC()` look nearly identical, with the only difference being the letter that is output; which can be set as a global variable.

---

Of course, for the first test, there is output `'A'` and the nothing but `'B'`s the rest of the screen.  I think I did something wrong and it's probably with the linked list.

I also realize now that while the last process created will be next to be scheduled (which is flawed anyway), the way in which I implemented the letter assignmet will keep them all in order.

---

OK, so I have a theory here.  Currently, my optimization level is `-O2` when I compile.  When I comment that out or disable optimizations, I get single `ABC` output and the a triple fault.  And finally, when I go with `-O4` I get the same results as `-O2`.  So my theory is that the compiler is optimizing the C code in a manner that breaks `cdecl`.  

Let me do a comparison of the assembly code that calls `SwitchToTask()`.  Recall that `SwitchToTask()` starts with the following code:

```asm
;;    * cdecl ensures that `eax`, `ecx`, and `edx` are saved on the stack
;; <snip>
SwitchToTask:
        push        ebx
        push        esi
        push        edi
        push        ebp
```

So, let's go verify that with `-O2`:

```asm
001002c0 <Schedule>:
  1002c0:       83 ec 18                sub    $0x18,%esp
  1002c3:       a1 00 2c 10 00          mov    0x102c00,%eax
  1002c8:       ff 70 08                pushl  0x8(%eax)
  1002cb:       e8 70 fd ff ff          call   100040 <SwitchToTask>
```

Well, this does not save anything.  But then again, `%eax` is only used and not required after the `call`.  So, now I need to go consider what calls `Schedule()`:

```asm
001000a0 <_Z7Processv>:
  1000a0:       53                      push   %ebx
  1000a1:       83 ec 08                sub    $0x8,%esp
  1000a4:       0f be 1d d0 17 10 00    movsbl 0x1017d0,%ebx
  1000ab:       8d 43 01                lea    0x1(%ebx),%eax
  1000ae:       a2 d0 17 10 00          mov    %al,0x1017d0
  1000b3:       8d b4 26 00 00 00 00    lea    0x0(%esi,%eiz,1),%esi
  1000ba:       8d b6 00 00 00 00       lea    0x0(%esi),%esi
  1000c0:       83 ec 0c                sub    $0xc,%esp
  1000c3:       53                      push   %ebx
  1000c4:       e8 37 02 00 00          call   100300 <WriteChar>
  1000c9:       e8 f2 01 00 00          call   1002c0 <Schedule>
  1000ce:       83 c4 10                add    $0x10,%esp
  1000d1:       eb ed                   jmp    1000c0 <_Z7Processv+0x20>
```

In this case `%ebx` is loaded with a value that is required to persist after the call to `Schedule` and `WriteChar()` was implemented without touching `%ebx`.  Therefore, the kernel will create process B and then process C.  It will then call Process itself which assigns itself a character and outputs an `'A'` (which is stored in `%ebx`).  `Schedule()` is called.  Then process B gets control (actually the last one created but it has not been assigned a character yet) and assigns itself `'B'` (also stored in `%ebx`) and outputs that character.  `Schedule()` is called again.  Finally C gets control, assigns itself `'C'` in `%ebx` and outputs that character.  `Schedule()` is called again.  Now, A gets control again, but `%ebx` is not updated to reflect its assigned character; it outputs `'C'` again.

Now, the same analysis with `-O0`:

The call to `SwitchToTask()`:

```asm
001002f3 <Schedule>:
  1002f3:       55                      push   %ebp
  1002f4:       89 e5                   mov    %esp,%ebp
  1002f6:       83 ec 08                sub    $0x8,%esp
  1002f9:       a1 20 13 10 00          mov    0x101320,%eax
  1002fe:       8b 40 08                mov    0x8(%eax),%eax
  100301:       83 ec 0c                sub    $0xc,%esp
  100304:       50                      push   %eax
  100305:       e8 36 fd ff ff          call   100040 <SwitchToTask>
```

Again, `%ebx` is not set and `%eax` is not needed.  So, the call to `Schedule()`:

```asm
001000a0 <_Z7Processv>:
  1000a0:       55                      push   %ebp
  1000a1:       89 e5                   mov    %esp,%ebp
  1000a3:       83 ec 18                sub    $0x18,%esp
  1000a6:       0f b6 05 e4 12 10 00    movzbl 0x1012e4,%eax
  1000ad:       89 c2                   mov    %eax,%edx
  1000af:       83 c2 01                add    $0x1,%edx
  1000b2:       88 15 e4 12 10 00       mov    %dl,0x1012e4
  1000b8:       88 45 f7                mov    %al,-0x9(%ebp)
  1000bb:       0f be 45 f7             movsbl -0x9(%ebp),%eax
  1000bf:       83 ec 0c                sub    $0xc,%esp
  1000c2:       50                      push   %eax
  1000c3:       e8 6c 02 00 00          call   100334 <WriteChar>
  1000c8:       83 c4 10                add    $0x10,%esp
  1000cb:       e8 23 02 00 00          call   1002f3 <Schedule>
  1000d0:       eb e9                   jmp    1000bb <_Z7Processv+0x1b>
```

This now stores the letter to print in `%eax`, which is clobbered by `Schedule()`.  However, this is actually stored on the stack properly so it should work -- I just need to figure out the triple fault to be able to prove it.

---

### 2019-Sep-26

Well I did find a stack symmetry problem:

```asm
        push        ebx
        push        esi
        push        edi
        push        ebp

;; -- snip

        pop         ebp
        pop         edi
        pop         esi
        pop         ebp         ;; oops!
```

This, of course, cleans up my problems with `-O0` as well.

And my theory was wrong -- which I should have known better!

---

### 2019-Sep-27

## Branch `step03`

This is an optional step, but one well worth taking on.  This step implements some time accounting functions.  For this to work for what I am working with, I would need to parse the ACPI tables for the APIC, parse the ACPI tables for the HPET, or just implement the timer to increment ticks at a specific interval.  For the first 2, I would need to fall back on the last just in case it was not supported anyway.  I am trying to remain simple here, so the timer interrupt is the way to go.

Note that I do not feel that the RTC will provide the granularity I need and therefore will not be used.

So, to do this, I will need to set up the PIC and the IDT.  I will need to make sure that the timer fires properly and update a global counter.

---

So, let's talk a little bit about the granularity we will measure.  Brendan recommends that we keep track of timings in a relatively high precision resolution.  In fact, the tutorial mentions nanosecond resolution -- 1 billionth of a second.  Therefore, 1 second would be 1,000,000,000 ticks.  At 32-bit unsigned integers, that would be just over 4 seconds before we roll over the clock.  64- or 128-bit integers require some fancy work to implement -- well, more than I am putting in for this test.

Ok, so microsecond resolution, or 1 millionth of a second.  That would be 4294 seconds before rolling over, or 71.6 minutes.

Finally, millisecond resolution, or 1 thousanth of a second.  That would be 4,294,967 seconds before rolling over, or 71,582 minutes, or 1193 hours, or 49 days.  While that would still not work for a "real" operating system, that will work for this test.  And millisecond resolution is just fine for me.

Now, for the 64-bit OS, you will likely have access to 128-bit registers and will be able to implement nanosecond resolution with a rollover time of about 584 years -- totally adequate.  With a 32-bit cpu, some software math libraries are needed for this and I do not want to go working those out.

By the way, for the real numbers in more production-ready 32-bit OS, I would probably need to implement a larger BCD (Binary Coded Decimal) arithmetic library and printing functions.

Now, it is very easy to align the timer with this counter resolution: 1ms, which I will do.

So, what this does, however, is set up for some serious inaccuracies in the timer code.  We will be switching tasks initially far more frequently than once per millisecond.  It will be luck of the draw on which task gets the "credit" for the CPU time.  For the purposes of this test, I can live with this.

---

### 2019-Sep-29

Now for the implementation part.  The first thing I need to do is register an ISR handler into the IDT table.  This now starts another conversation -- how far to I go with these interrupt handlers?

Well, this is a purpose-built test, so I do not plan on going very far.  My only plan at this point in time is to register the ISR for IRQ0 and leave it at that.  Until the IDT is constructed properly, the system will triple-fault anyway so there is nothing to be gained to have a ready-made exception handler for every possible contingency.

[Century-OS](https://github.com/eryjus/century-os) also has the ability to register and de-register a custom handler into a jump table.  I am not going to do that either.  I am going to register the handler into the IDT directly -- something I may decide to go back and do later in [Century-OS](https://github.com/eryjus/century-os).

So, let's review.  I already have an IDT registered:

```asm
        align       8
idtSize:
        dw          0x7f
idtLoc:
        dd          0x17f000
```

So, the IDT is located at `0x17f000`.  Again, paging is not enabled, so that is not a factor.  I just need a pointer to that memory address, a structure definition, and a function to write the fields of that structure.

I code....

---

OK, I have that done, but not tested.  I now need a handler that will increment the timer.  This should be relatively simple as all I need to do is increment a value in memory and then `iret`.  No need to save any registers I do not touch.

---

Finally, I need to initialize the PIT and PIC.

---

### 2019-Oct-04

Today, I pick up where I left off setting up the PIC and PIT.  Since this is purpose-built, I will not be developing proper driver and no abstraction.

---

OK, I did manage to get some coding done today.  I do need to be able to issue an EOI.  I save that for tomorrow.

---

### 2019-Oct-05

I have finished the cleanup for the counter.  Interrupts should be able to fire and the counter should increment in ticks.

---

So, after an initial problem with getting the 8259 data and command ports mixed up, I am now triple faulting with the vector being out of range for the IDT.  

```
00143483792e[CPU0  ] interrupt(): vector must be within IDT table limits, IDT.limit = 0x7f
00143483792e[CPU0  ] interrupt(): gate descriptor is not valid sys seg (vector=0x0d)
00143483792e[CPU0  ] interrupt(): gate descriptor is not valid sys seg (vector=0x08)
00143483792i[CPU0  ] CPU is in protected mode (active)
00143483792i[CPU0  ] CS.mode = 32 bit
00143483792i[CPU0  ] SS.mode = 32 bit
00143483792i[CPU0  ] EFER   = 0x00000000
00143483792i[CPU0  ] | EAX=00000004  EBX=00010000  ECX=00000007  EDX=0000000a
00143483792i[CPU0  ] | ESP=00180ff4  EBP=00000000  ESI=00000000  EDI=00000000
00143483792i[CPU0  ] | IOPL=0 id vip vif ac vm RF nt of df IF tf sf zf af pf cf
00143483792i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
00143483792i[CPU0  ] |  CS:0008( 0001| 0|  0) 00000000 ffffffff 1 1
00143483792i[CPU0  ] |  DS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00143483792i[CPU0  ] |  SS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00143483792i[CPU0  ] |  ES:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00143483792i[CPU0  ] |  FS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00143483792i[CPU0  ] |  GS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00143483792i[CPU0  ] | EIP=001001a1 (001001a1)
00143483792i[CPU0  ] | CR0=0x60000011 CR2=0x00000000
00143483792i[CPU0  ] | CR3=0x00000000 CR4=0x00000000
(0).[143483792] [0x0000001001a1] 0008:00000000001001a1 (unk. ctxt): call .-198 (0x001000e0)   ; e83affffff
00143483792p[CPU0  ] >>PANIC<< exception(): 3rd (13) exception with no resolution
```

So, `IDT.limit` is `0x7f`, and the limit should be `(4 * 256) - 1` or `0x3ff`.

So, with that fixed, I have a new and improved triple fault:

```
00144406767e[CPU0  ] interrupt(): gate.type(12) != {5,6,7,14,15}
00144406767e[CPU0  ] interrupt(): gate.type(12) != {5,6,7,14,15}
00144406767e[CPU0  ] interrupt(): gate descriptor is not valid sys seg (vector=0x08)
00144406767i[CPU0  ] CPU is in protected mode (active)
00144406767i[CPU0  ] CS.mode = 32 bit
00144406767i[CPU0  ] SS.mode = 32 bit
00144406767i[CPU0  ] EFER   = 0x00000000
00144406767i[CPU0  ] | EAX=00000000  EBX=00000000  ECX=00000034  EDX=00000000
00144406767i[CPU0  ] | ESP=001003b0  EBP=00000000  ESI=00000005  EDI=cccccccd
00144406767i[CPU0  ] | IOPL=0 id vip vif ac vm RF nt of df IF tf sf ZF af PF cf
00144406767i[CPU0  ] | SEG sltr(index|ti|rpl)     base    limit G D
00144406767i[CPU0  ] |  CS:0008( 0001| 0|  0) 00000000 ffffffff 1 1
00144406767i[CPU0  ] |  DS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00144406767i[CPU0  ] |  SS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00144406767i[CPU0  ] |  ES:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00144406767i[CPU0  ] |  FS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00144406767i[CPU0  ] |  GS:0010( 0002| 0|  0) 00000000 ffffffff 1 1
00144406767i[CPU0  ] | EIP=001003bd (001003bd)
00144406767i[CPU0  ] | CR0=0x60000011 CR2=0x00000000
00144406767i[CPU0  ] | CR3=0x00000000 CR4=0x00000000
(0).[144406767] [0x0000001003bd] 0008:00000000001003bd (unk. ctxt): int3                      ; cc
00144406767p[CPU0  ] >>PANIC<< exception(): 3rd (13) exception with no resolution
```

So, it looks like I am not programming the vector properly.  This will be a little harder to track down.

First, what is at `eip`?

```asm
  1003b0:       c3                      ret    
  1003b1:       8d b4 26 00 00 00 00    lea    0x0(%esi,%eiz,1),%esi
  1003b8:       8d b4 26 00 00 00 00    lea    0x0(%esi,%eiz,1),%esi
  1003bf:       90                      nop
```

So, it looks like I am executing off the end of someting.  I will need to see the instructions leading up to that.

---

I had a bug in my `WriteDec()` code.  With that fixed, everything works.

So, now I need to collect the number of ticks and store them in the processes at task swap.

This appears to be working properly.  I think I am ready to commit this code.

## Branch `step04`

In this step we will undo several of the things we did in step02 and replace them with constructs that are geared for the final kernel scheduler.

The tasks we are going to complete in this step are:
* break the circular linked list and have `currentPCB` actually point to the current task on the CPU
* create a new set of variables for the ready-to-run processes which mark the head and tail of the list
* create a set of states that are used to indciate what the task is waiting for and therefore where is it located.

### 2019-Oct-06

Well, I was able to get most of the Step 4 code written yesterday and I was able to test and quickly address the small bug I had.  This change came together very quickly.

### 2019-Oct-24

I committed the `step04` branch today.

## Branch `step05`

This next step will add some basic locking functionality to the scheduler.  These rudamentary locks will form the foundation for a more robust lock later on.  It's really about laying the fountation for later enhancement.

## Branch `step06`

This branch will create the basic for blocking a process voluntarily.  But not today.

### 2019-Oct-25

The coding for this section went quick enough, but there is a problem where everything is still getting CPU time.

However, what is interesting, is as cycles through the processes went on several did drop off (starting with A-N and then ending with A-I at the end of the page).  Another interesting point is that the letters drop off all at once.

Now, if I move the process I am trying to release, I get a shorter list of resulting characters (say, A-E for example).

---  

It turns out that I had an extra state change in `Schedule()` to set the departing process state to be `READY`.  Commenting this line out made it all work:

```C
void Schedule(void) 
{
    PCB_t *next = NextReady();

    if (next) {
//        AddReady(currentPCB);
        SwitchToTask(next);
    }
}
```

So, we just need a little cleanup....

## Step 7 -- sleep requirements

This step in the tutoral is a discussion about the locks that are required for a multiple processes and in particular when multiple processes become unblocked, we need to be able to let the scheduler make the best scheduling decision.

## Branch `step08`

So, as a result of the discussion from #7, we need to improve the locking mechanisms.  

### 2019-Oct-26

I think the text related to this additional locking is confusing.  You absolutely have to read it carefully and for content.  In short, we will be adding a second locking mechanism (with different rules) for locking other things than the scheduler structures.  This lock will be used outside the core of the scheduler for obtaining and releasing locks.

This, in effect, leaves the scheduler lock as a purposeful lock that does not follow the exact same rules as other locks.

Now, with that said, there is a contridiction in the tutorial:

#### From Step 7:
> ..., we want a way to tell the scheduler to postpone task switches while the kernel is holding any lock (and do the postponed task switch/es when we aren't holding a lock any more), and **we want to do this for all locks including the scheduler's lock.**

#### From Step 8:
> We already have code for locking and unlocking the scheduler; but you can't really use the same lock that the scheduler uses because you don't want the scheduler to disable task switching every time it needs to do a task switch, so we're going to have a different lock for miscellaneous stuff instead.


### 2019-Nov-01

I am still trying to decipher if I am supposed to create a wrapping lock or if I am supposed to replace the scheduler lock with this other one.

OK, finally!!  Later on, `terminate_task()` uses both locks.  With that, I now know I need to keep them both.

So, Brendan calls his function `lock_stuff()`.  I think this is a little misleading.  I will name my own `LockAndPostpone()` since the purpose is not only to obtain a lock but also to postpone process changes.  Normally, I would make it so that we would pass in which lock to obtain, but in keeping with Brendan's thoughts, we will keep it simple.

This is actually, after all the analysis, a trivial change in preparation for the next step.

## Branch `step09`

Now, we have the foundation to implement the sleep functions.  The tutorial has a granularity of 1ns (or 1/1,000,000,000 or a second), but for my implementation I have 1ms granularity.  So for this purpose-built test, I am going to stick with 1ms granularity.  So, there will be just `Sleep()` and `SleepUntil()` implementation; and since `Sleep()` will call `SleepUntil()`, that is the function I need to focus on.

### 2019-Nov-03

Today I start debugging my timer.  Nothing appears to be working so I have to set up some debugging code to work it all out.

So, it looks like I get 1 interrupt and then nothing.  So, I think I need to EOI before I call the handler.

### 2019-Nov-04

OK, I thought this was working before and I was able to confirm: when I comment out all the calls to create a process and only look for a timer, everything works properly.  So my timer code is correct as far as I can tell.

I need to find out why creating a new process kills the timer.

And as expected, the timer does not fire again when I create a new process.

### 2019-Nov-05

OK, so I want to identify what is going on with when creating a process and why interrupts are not being re-enabled.  Looking at the logs, I am not able to see that the new process is calling `ProcessStartup()`.

So, reversing the order of these lines to this:

```C
    PUSH(tos, (unsigned int)ent);        // entry point
    PUSH(tos, (unsigned int)ProcessStartup);  // startup function
```

... fixed that problem.  Now at least gets the timer to fire.

However, sleep is not working.  Or at least the processes are not waking up.

So, the problem happened to be in `UnblockProcess()` where it was attempting to do an immediate change to a process when task changes were postponed.  Cleaning this up (to only make the process ready) solved this last problem.

So, with this, I think I am ready to commit.



