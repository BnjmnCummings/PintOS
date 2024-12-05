# PintOS

My solution to Stanford University's [PintOS](https://en.wikipedia.org/wiki/Pintos) project, submitted as part of my Operating Systems module at Imperial College London. This project involved implementing some fundamental operating system features including but not limited to: thread scheduling and synchronisation, system call handling, file handling and virtual memory handling. Concluding these tasks meant clearing our implementation of all race conditions and memory leak, allowing us to have a 'water-tight' environment to run user-programs.

>## Pre-requisites for running PintOS
>
>- [**GCC**](http://gcc.gnu.org/): Version 5.4 or later is preferred. Version 4.0 or
>later should work. If the host machine has an 80x86 processor (32-bit or 64-bit), then GCC
>should be available via the command gcc; otherwise, an 80x86 cross-compiler should be
>available via the command i386-elf-gcc. If you need a GCC cross-compiler, but one is
>not already installed on your system, then you will need to search online for an up-to-date
>installation guide.
>
>- [**GNU binutils**](http://www.gnu.org/software/binutils/): PintOS uses the
>Unix utilities addr2line, ar, ld, objcopy, and ranlib. If the host machine does not have
>an 80x86 processor, then versions targeting 80x86 should be available to install with an
>‘i386-elf-’ prefix.
>
>- [**Perl**](http://www.perl.org): Version 5.20.0 or later is preferred. Version 5.6.1
>or later should work.
>
>- [**GNU make**](http://www.gnu.org/software/make/): Version 4.0 or later is
>preferred. Version 3.80 or later should work.
>
>- [**QEMU**](http://fabrice.bellard.free.fr/qemu/): The QEMU emulator required to run PintOS is qemu-system-i386 which is part of the qemu-system package on
>most modern Unix platforms. We recommend using version 2.10 or later, but at least version 2.5

If you like PintOS, why not try our... [ARM emulator and assembler](https://github.com/BnjmnCummings/arm-v8-emulator-and-assembler)!?
