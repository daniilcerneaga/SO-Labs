# Lab 2 — Building the submission

`user/sleep.c` is the new xv6 program. `notes.md` contains the required Observe answers, and `report.md` contains the complete report, also supplied as DOCX and PDF. The included `Makefile` adds the program to `UPROGS`.

The tested xv6 revision is `06aad25c735fd3159bdfae5680be4eab7b1668b2`, using RISC-V GCC 15.2.0 and QEMU 10.2.1. No kernel or header changes are required.

The Makefile is adapted from MIT's xv6 source; its copyright and permission notice are preserved in `LICENSE.xv6`.

From the repository root, run in Linux with Git, make, a RISC-V cross-compiler and QEMU installed:

```bash
LAB2="$(pwd)/Lab2"
XV6_BUILD=$(mktemp -d)
git clone https://github.com/mit-pdos/xv6-riscv "$XV6_BUILD/xv6"
cd "$XV6_BUILD/xv6"
git checkout 06aad25c735fd3159bdfae5680be4eab7b1668b2
cp "$LAB2/user/sleep.c" user/sleep.c
cp "$LAB2/Makefile" Makefile
make qemu CPUS=1
```

At the xv6 prompt, `sleep 10` waits for ten clock ticks and returns silently. `sleep` without an argument prints `usage: sleep <ticks>`. Leave QEMU with Ctrl-A followed by X.

In the tested revision the waiting syscall is named `pause`. The program also contains a compatibility branch for headers using the older `sleep` name; that branch was not separately tested. The report includes the measured delays and argument-validation results.
