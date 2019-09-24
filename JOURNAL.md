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

```
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

