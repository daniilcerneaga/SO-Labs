# Lab 8 — Observe answers

**Daniil Cerneaga, FAF-243.** See report.md for commands, outputs and the full explanation.

## Part 1. run

1. Linux uses fork, execlp and waitpid; xv6 uses fork, exec and wait. Both create a child, replace its program and collect its exit result. Linux's PATH-search wrapper and wait-status encoding differ from xv6's interfaces.
2. echo prints “hello from the child” after exec succeeds. run itself prints only the identity/error/status lines.
3. The failed-exec branch chooses exit(1) in xv6 and exit(127) in the Linux example. These values come from the programs' explicit exit calls.

## Part 2. pingpong

1. Two one-way pipes separate the parent-to-child request from the child-to-parent reply and avoid either process reading its own message.
2. read(c2p[0], &byte, 1) blocks the parent until the reply arrives or the stream closes/fails. wait later reaps the child; it is not what makes the parent's reply read wait.
