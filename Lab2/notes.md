# Lab 2 — Observe answers

**Daniil Cerneaga, FAF-243.** The tested xv6 revision is `06aad25c735fd3159bdfae5680be4eab7b1668b2`. See `report.md` for the build, implementation, command outputs and test results.

## Part 2. Using xv6

1. Three supplied programs are `ls` (list names and metadata), `cat` (copy file contents to standard output), and `wc` (count lines, words and bytes). They are user-space programs, not shell built-ins. `wc README` returned `48 336 2441 README`.
2. A working pipe requires process creation/execution and inter-process communication. For `ls | grep c`, the shell starts the two programs and connects their file descriptors to the write and read ends of a kernel pipe. It closes unused ends and waits for its children; a reader can observe EOF once all writers close their descriptors.
3. The xv6 shell offers the same basic command, pipe and redirection model as Bash, with a much smaller interface and without Bash's variable expansion, filename globbing and command-history facilities.

Selected real output:

```text
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

## Part 3. Source reading

1. `user/cat.c` uses these system calls: `open` obtains a readable descriptor for a named file; `read` copies bytes from a descriptor into a user buffer; `write` sends bytes from that buffer to descriptor 1; `close` releases an opened descriptor; and `exit` terminates the process with a status. With no file argument, `cat` reads descriptor 0. `fprintf` is a formatting-library routine that ultimately uses `write`, rather than a separate syscall.
2. `sys_read(void)` is at line 69 of `kernel/sysfile.c` in the tested revision; its return-type line is 68 and its body begins at line 70. The handler retrieves the file descriptor, user buffer address and length, then delegates to `fileread`. `sys_write(void)` is at line 83.
3. `kernel/` implements privileged supervisor-mode services and resource management, whereas `user/` contains unprivileged programs which request those services through system calls.

Evidence:

```text
$ grep -n 'sys_read\|sys_write' kernel/sysfile.c
69:sys_read(void)
83:sys_write(void)
```

The pinned source declares `pause(int)` rather than `sleep(int)` in `user/user.h`. The executable submitted as `sleep` uses that declaration; the kernel and its headers are not modified.
