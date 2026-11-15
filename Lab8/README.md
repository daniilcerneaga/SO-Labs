# Lab 8 — Build and run

The submission adds user/run.c, user/pingpong.c and the optional user/primes.c to the pinned xv6 source. From the course repository root in Linux, with Git, make, a RISC-V cross-compiler and QEMU installed:

```bash
LAB8="$(pwd)/Lab8"
XV6_BUILD=$(mktemp -d)
git clone https://github.com/mit-pdos/xv6-riscv "$XV6_BUILD/xv6"
cd "$XV6_BUILD/xv6"
git checkout 06aad25c735fd3159bdfae5680be4eab7b1668b2
cp "$LAB8"/user/*.c user/
cp "$LAB8/Makefile" Makefile
make qemu CPUS=1
```

At the xv6 prompt run `run`, `pingpong` and `primes`. Leave QEMU with Ctrl-A followed by X. notes.md contains the required Observe answers, and report.md/DOCX/PDF contain the full report. The Makefile is adapted from MIT xv6; its copyright and permission notice is in LICENSE.xv6.
