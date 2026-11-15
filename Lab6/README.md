# Lab 6 — Build and run

The submission adds mystat and find to xv6 revision `06aad25c735fd3159bdfae5680be4eab7b1668b2`. Install Git, make, a RISC-V cross-compiler and RISC-V QEMU in Linux. From the course repository root:

```bash
LAB="$(pwd)/Lab6"
XV6_BUILD=$(mktemp -d)
git clone https://github.com/mit-pdos/xv6-riscv "$XV6_BUILD/xv6"
cd "$XV6_BUILD/xv6"
git checkout 06aad25c735fd3159bdfae5680be4eab7b1668b2
cp "$LAB"/user/*.c user/
cp "$LAB/Makefile" Makefile
make qemu CPUS=1
```

Exit QEMU with Ctrl-A followed by X. The supplied Makefile is adapted from MIT xv6; its copyright and permission notice is in `LICENSE.xv6`. The report is supplied as Markdown, DOCX and PDF.

Inside xv6, use `mystat README . console`, `find . README`, or `find . README -n`. The finder prints exact last-component matches and supports deep trees using an explicit DFS stack. The report contains the required fresh-image example and observed inode/link results.
