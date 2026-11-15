# Laboratory Work No. 2 — Meet the OS You Will Build

**Student:** Daniil Cerneaga, FAF-243  
**Supervisors:** Marius Dumitrescu; Reițman Patricia  
**Course:** Operating Systems, FCIM, Technical University of Moldova  
**Date:** 12 November 2026

## Purpose and build environment

This laboratory connects the external view of Linux from Lab 1 with the source code of a small operating system. The tasks are to compile and boot xv6, use its command-line programs, locate the kernel side of file-reading requests, and add a program that waits for a specified number of clock ticks.

The host environment was Ubuntu 26.04.1 under WSL2. xv6 itself ran as a separate RISC-V guest inside QEMU; it was not a Linux process environment with the same system calls as the host. The build used the course setup script's pinned upstream revision, `06aad25c735fd3159bdfae5680be4eab7b1668b2`, RISC-V GCC 15.2.0 and QEMU 10.2.1. QEMU was configured with one hart (CPU) and 128 MiB RAM. These settings are independent of the approximately 3.8 GiB visible to the Linux host in Lab 1.

A clean source tree at the pinned commit was used for the build. The added program is `user/sleep.c`; the supplied `Makefile` registers it in `UPROGS`. The kernel and its headers are unchanged.

## Part 1. Build and boot

The kernel and filesystem image were built with `make -j2 kernel/kernel fs.img`, then booted using `make qemu CPUS=1`. The final image-creation lines and the boot messages were:

```text
nmeta 47 (boot, super, log blocks 31, inode blocks 13, bitmap blocks 1) blocks 1953 total 2000
balloc: first 1020 blocks have been allocated
balloc: write bitmap block at sector 46

xv6 kernel is booting

init: starting sh
$
```

*Listing 1.1 — Successful filesystem-image creation and xv6 boot.*

The filesystem creation step succeeded, the kernel reached its init process, and the shell accepted commands. With `CPUS=1`, no messages announcing additional harts were expected. QEMU was exited using Ctrl-A followed by X after the commands completed.

### Background and sources

The procedure follows the course handout, “Lab 2 — Meet the OS You Will Build”; the revision is pinned by the supplied setup script. Source-reading answers refer to [MIT's xv6 source at commit 06aad25](https://github.com/mit-pdos/xv6-riscv/tree/06aad25c735fd3159bdfae5680be4eab7b1668b2). A cross-compiler is needed because the host is x86-64 while the kernel and its programs target RISC-V. The guest kernel provides services to user programs through system calls; it is separate from the Linux kernel hosting QEMU.

<!-- pagebreak -->

## Part 2. Use xv6 as Unix

All required commands were run: `ls`, `cat README`, `echo hello xv6`, `ls | grep c`, and `wc README`. The new `sleep` program appeared in the directory listing as an ordinary file. Reading `README` printed the upstream description and build instructions. Selected output:

```text
$ ls
.              1 1 1024
..             1 1 1024
README         2 2 2441
cat            2 3 36728
echo           2 4 35592
sleep          2 5 36256
...
$ echo hello xv6
hello xv6
$ ls | grep c
cat            2 3 36728
echo           2 4 35592
wc             2 18 37728
sync           2 23 34944
console        3 24 0
$ wc README
48 336 2441 README
```

*Listing 2.1 — Files, a pipeline and the word count inside xv6.*

The `ls` excerpt is shortened with an ellipsis. The pipeline selects any line containing `c`, which is why `echo` and `console` appear as well as `cat` and `wc`.

### Observe 1 — Three supplied programs

`ls` lists directory entries, `cat` reads files and writes their contents to standard output, and `wc` counts lines, words and bytes. The last command measured 48 lines, 336 words and 2,441 bytes in `README`. These utilities live in `user/`; they are executed by the shell rather than implemented as built-in shell commands.

### Observe 2 — Features required for a pipe

The two essential capabilities are running multiple processes and communicating between them. The shell creates a pipe and uses `fork`/`exec` to launch the commands. It connects the producer's standard output and the consumer's standard input using file descriptors. The kernel supplies the buffer and blocking behaviour, so `grep` can wait for bytes without accessing `ls` memory. Closing unused pipe ends also matters: the consumer sees end-of-file only once no writer remains.

### Observe 3 — Comparison with the Linux shell

The xv6 shell retains Bash's basic model of executing programs and composing pipes and redirections, while leaving out conveniences such as history, filename globbing and shell-variable expansion.

<!-- pagebreak -->

## Part 3. Read both sides of an I/O request

The inspection read the beginning of `user/ls.c`, the complete `user/cat.c`, and the declarations in `user/user.h`. The kernel handlers were located with the following command:

```text
$ grep -n 'sys_read\|sys_write' kernel/sysfile.c
69:sys_read(void)
83:sys_write(void)
```

*Listing 3.1 — Locations of the kernel read and write handlers.*

### Observe 1 — System calls in cat

`open(path, O_RDONLY)` asks the kernel for a descriptor that refers to an opened file. `read(fd, buf, sizeof(buf))` requests bytes into a user-space buffer; its positive return value tells `cat` how many bytes were read. `write(1, buf, n)` sends that many bytes to standard output, which may be a console, a file or a pipe depending on the shell's setup. `close(fd)` releases an opened descriptor, and `exit(status)` ends the process. With no filename, `cat` uses descriptor 0 instead of opening a named file.

The code's `fprintf(2, ...)` calls are library formatting operations rather than additional system calls. They report errors through the `write` interface. This distinction separates the user-side convenience routine from the actual kernel service.

### Observe 2 — Where sys_read is implemented

In this exact revision, the function name `sys_read(void)` is on line 69 of `kernel/sysfile.c`; the `uint64` return type is on line 68 and the opening brace is on line 70. Inside the handler, `argaddr` obtains the destination user address, `argint` obtains the count, and `argfd` validates the descriptor and finds its open-file structure. The handler then returns `fileread(f, p, n)`. Line numbers belong to this commit and can change in another version.

### Observe 3 — Kernel versus user code

Code in `kernel/` runs in privileged supervisor mode and implements scheduling, address spaces, devices and files; code in `user/` runs without those privileges and uses system calls to request services.

The declarations in `user/user.h`, the generated user syscall stubs, and the handlers in `kernel/` describe different stages of the same request. Reading these alongside `cat.c` makes the boundary more concrete than simply observing a Linux command's final output. The six short Observe answers are also collected in `notes.md`.

<!-- pagebreak -->

## Part 4. Implement sleep

The new executable accepts one non-negative decimal tick count. It rejects missing, empty, non-numeric, negative, overflowing or extra arguments with `usage: sleep <ticks>` on descriptor 2 and exit status 1. A valid request waits through the existing kernel service and exits with status 0. Zero is accepted as a zero-length wait. The parser checks the range before multiplying, so a value above the maximum signed 32-bit integer cannot silently wrap.

The lab sheet uses the old syscall name `sleep`, but the course's pinned headers declare `pause(int)`, with `SYS_pause` defined in `kernel/syscall.h`. The compatibility selection below keeps the executable named `sleep` and chooses the declaration provided by the headers. It does not rename a kernel syscall or change `user/user.h`. The `pause` branch is the one built and tested here; the older-API branch has not been tested on another revision.

```c
#ifdef SYS_pause
  int result = pause(ticks);
#else
  int result = sleep(ticks);
#endif
  if(result < 0){
    fprintf(2, "sleep: interrupted\n");
    exit(1);
  }
  exit(0);
```

*Listing 4.1 — Selecting the syscall declared by the xv6 headers.*

The full implementation is `user/sleep.c`. Its build registration is a tab-indented entry immediately after `_echo` in `UPROGS`:

```makefile
UPROGS=\
	$U/_cat\
	$U/_echo\
	$U/_sleep\
```

*Listing 4.2 — Registering sleep in the user-program build list.*

There is no comment after a continuation backslash in the submitted Makefile. Building this program alongside the existing utilities includes it in `fs.img`, so the xv6 shell can execute it. The ordinary shell checks produced:

```text
$ sleep 10
$ sleep
usage: sleep <ticks>
$ sleep 25
$ sleep abc
usage: sleep <ticks>
```

*Listing 4.3 — Valid calls and usage-error handling in the xv6 shell.*

A lack of output after `sleep 10` alone does not demonstrate that it waited correctly, so the next step measured the tick counter and child exit status.

<!-- pagebreak -->

## Verification with an independent xv6 process

A separate test process recorded `uptime()`, forked, executed `sleep`, waited for its exit status, then read `uptime()` again. This measured the executable through public system calls. The submitted Makefile registers the lab program without the test observer.

| Case | Expected status | Actual status | Elapsed ticks |
| --- | --- | --- | --- |
| `sleep 0` | 0 | 0 | 0 |
| `sleep 10` | 0 | 0 | 10 |
| `sleep 25` | 0 | 0 | 25 |
| No argument | 1 | 1 | 0 |
| `sleep abc` | 1 | 1 | 0 |
| `sleep -1` | 1 | 1 | 0 |
| `sleep 2147483648` | 1 | 1 | 0 |
| `sleep 10 20` | 1 | 1 | 0 |
| Empty argument | 1 | 1 | 0 |

*Table 5.1 — Exit statuses and elapsed guest ticks for nine test cases.*

Selected lines from the actual observer output:

```text
PASS: zero; status=0; elapsed=0 ticks; minimum=0
PASS: ten; status=0; elapsed=10 ticks; minimum=10
PASS: twentyfive; status=0; elapsed=25 ticks; minimum=25
sleepcheck: 0 failures
```

*Listing 5.1 — Timing checks and the final test result.*

The checks require at least the requested number of ticks, rather than imposing an exact wall-clock duration: scheduling and emulation can delay a process after it becomes runnable. The measurements concern xv6 clock ticks, not Linux seconds. All nine cases passed.

The successful build and targeted tests validate this program and the required interaction with xv6; they are not a claim that the optional full `usertests -q` kernel suite was run. The optional `pingpong` extension is not included.

### Conclusion

The completed run connected command-line behaviour with both sides of a system call: `cat` requests I/O in user mode, and `sys_read` validates and forwards that request inside the kernel. Adding `sleep` required a user program and a build-list entry, while the waiting service already existed. Measuring 10 and 25 elapsed ticks confirmed that the new executable requested a real kernel-managed delay rather than only returning silently to the shell.
