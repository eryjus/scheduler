# Scheduler Test Journal

As always, I am going to produce a journal of my efforts with a particular project.  In this case, the project is to work out the kinks with some purpose-built code for the scheduler.  

Yesterday, I spent the time to get a boot image built and able to boot and print a *welcome* message.  It is the most trivial "Hello World" application that can be built with a multiboot bootloader.

Be sure to see [README.md](README.md) and [LICENSE.md](LICENSE.md) for more information.


### 2019-Sep-22

So, since I am working from Brandan's tutorial in an isolated environment, I will be working as closely to his tutorial as I can.  I will likely pull small bits of code from Century-OS, but just to save me some typing.  I will strip the code as necessary to match Brendan's tutorial.

Part 1 is to write a function called `Switch_to_task()`.  In CenturyOS, this function was `ProcessSwitch()`.  It was written in assembly language as it should be -- it is architecture dependent.  Also started in this step is the data structure needed to keep track of what a "Process" is.

I wanat to call out here the Brendan is assuming that paging is enabled.  In my tests, it is not.  Therefore, `cr3` is not a relevant register to change.  I will write the code to update it, but in reality if I write my code properly it will never have to change (all `0` values).  If for some reason it does change, the `PG` flag is not set so there should be no ramifications from the change -- there should be not `#PF` or `#GP` to worry about.

