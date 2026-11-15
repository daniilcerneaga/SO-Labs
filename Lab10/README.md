# Lab 10 — xv6 with trace

`xv6/` is the complete buildable source tree based on MIT xv6 revision `06aad25c735fd3159bdfae5680be4eab7b1668b2`, with the trace syscall and user wrapper added. Its upstream permission notice is in `xv6/LICENSE`.

In Linux with make, a RISC-V cross-compiler and QEMU installed:

```bash
cd Lab10/xv6
make qemu CPUS=1
```

At the xv6 prompt:

```text
trace 32 grep hello README
trace 2147483647 grep hello README
trace 2 usertests forkforkfork
trace 65536 echo hi
trace 2129920 grep hello README
```

Leave QEMU with Ctrl-A followed by X. Syscall 22 remains sync; trace uses 23 and prints through kernel printk. Masks are stored per process, copied during fork, retained by exec, and reset when process slots are allocated/released. The read-only selection prints 1023, 965, 453 and 0 for the pinned README.

The submitted tree contains source files, not compiled binaries or disk images. `make clean` removes generated build products if a new filesystem image is wanted. `notes.txt` has the required Observe answers; report.md and the final DOCX/PDF contain the full experiments and results.
