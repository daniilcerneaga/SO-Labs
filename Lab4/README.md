# Lab 4 — Build and run

The submission adds sleep and args to xv6 revision `06aad25c735fd3159bdfae5680be4eab7b1668b2`. Install Git, make, a RISC-V cross-compiler and RISC-V QEMU in Linux. From the course repository root:

```bash
LAB="$(pwd)/Lab4"
XV6_BUILD=$(mktemp -d)
git clone https://github.com/mit-pdos/xv6-riscv "$XV6_BUILD/xv6"
cd "$XV6_BUILD/xv6"
git checkout 06aad25c735fd3159bdfae5680be4eab7b1668b2
cp "$LAB"/user/*.c user/
cp "$LAB/Makefile" Makefile
make qemu CPUS=1
```

Exit QEMU with Ctrl-A followed by X. The supplied Makefile is adapted from MIT xv6; its copyright and permission notice is in `LICENSE.xv6`. The report is supplied as Markdown, DOCX and PDF.

For the Linux argument printer, compile `args_linux.c` with `gcc -Wall -Wextra -o args args_linux.c`, then run `./args` with arguments in a folder containing some .c files. The xv6 sleep program uses the pinned revision's `pause` syscall.
