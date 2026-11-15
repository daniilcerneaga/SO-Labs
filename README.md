# Operating Systems — laboratory work

**Daniil Cerneaga, FAF-243**  
Technical University of Moldova, FCIM  
Supervisors: Marius Dumitrescu; Reițman Patricia

Each folder contains the submitted files and a report in Markdown, DOCX and PDF. Reports include the answers to the Observe questions, commands, recorded results and conclusions.

| Lab | Topic | Report |
| --- | --- | --- |
| [1](Lab1/) | The OS as a Resource Manager | [PDF](Lab1/Lab1_Daniil_Cerneaga_FAF-243.pdf) · [Markdown](Lab1/report.md) |
| [2](Lab2/) | Meet the OS You Will Build | [PDF](Lab2/Lab2_Daniil_Cerneaga_FAF-243.pdf) · [Markdown](Lab2/report.md) |
| [3](Lab3/) | Shell, Pipes and Text Filters | [PDF](Lab3/Lab3_Daniil_Cerneaga_FAF-243.pdf) · [Markdown](Lab3/report.md) |
| [4](Lab4/) | The xv6 Shell from the Outside | [PDF](Lab4/Lab4_Daniil_Cerneaga_FAF-243.pdf) · [Markdown](Lab4/report.md) |
| [5](Lab5/) | The File System with Your Own Hands | [PDF](Lab5/Lab5_Daniil_Cerneaga_FAF-243.pdf) · [Markdown](Lab5/report.md) |
| [6](Lab6/) | Inside the xv6 File System | [PDF](Lab6/Lab6_Daniil_Cerneaga_FAF-243.pdf) · [Markdown](Lab6/report.md) |
| [7](Lab7/) | Processes, Redirection and Pipes | [PDF](Lab7/Lab7_Daniil_Cerneaga_FAF-243.pdf) · [Markdown](Lab7/report.md) |
| [8](Lab8/) | The Same Calls, Inside Your Own OS | [PDF](Lab8/Lab8_Daniil_Cerneaga_FAF-243.pdf) · [Markdown](Lab8/report.md) |
| [9](Lab9/) | Watching System Calls with strace | [PDF](Lab9/Lab9_Daniil_Cerneaga_FAF-243.pdf) · [Markdown](Lab9/report.md) |
| [10](Lab10/) | Add Your Own System Call: trace | [PDF](Lab10/Lab10_Daniil_Cerneaga_FAF-243.pdf) · [Markdown](Lab10/report.md) |
| [11](Lab11/) | The Race and the Semaphore | [PDF](Lab11/Lab11_Daniil_Cerneaga_FAF-243.pdf) · [Markdown](Lab11/report.md) |
| [12](Lab12/) | Producer, Consumer and the Other Tools | [PDF](Lab12/Lab12_Daniil_Cerneaga_FAF-243.pdf) · [Markdown](Lab12/report.md) |

## Linux exercises

Labs 1, 3, 5, 7, 9, 11 and 12 use Linux. The recorded environment was Ubuntu under WSL2. Lab3 scripts can be run with `bash oneliners.sh` and `bash report.sh access.log` after creating the log specified in the assignment. Direct execution with `./report.sh` additionally requires `chmod +x report.sh`.

Compile the Lab7 and Lab9 programs from their respective folders with:

```sh
gcc -Wall -Wextra -Werror -o run run.c
gcc -Wall -Wextra -Werror -o syscalls syscalls.c
```

The other Lab7 sources use the same flags and their own executable names. The original zombie experiment deliberately waits for 20 seconds.

The Lab11 and Lab12 programs require `-pthread`. For example, inside Lab11:

```sh
gcc -Wall -Wextra -Werror -O0 -pthread -o race race.c
gcc -Wall -Wextra -Werror -O0 -pthread -o mutex mutex.c
```

Keep optimization disabled for the race experiment. `race.c` intentionally contains a data race; the protected counters, ordering program and parking simulation are separate files. In Lab12, the submitted `deadlock.c` is the fixed version. Reports document the deliberately broken experimental variants as well.

## xv6 exercises

Labs 2, 4, 6 and 8 contain additions to MIT xv6, together with a Makefile and instructions for building against revision `06aad25c735fd3159bdfae5680be4eab7b1668b2`. Follow the README in each folder. These builds need Linux, make, a RISC-V cross-compiler and RISC-V QEMU.

Lab10 contains the complete modified xv6 tree and can be built directly:

```sh
cd Lab10/xv6
make qemu CPUS=1
```

Exit QEMU with Ctrl-A followed by X. Upstream license notices accompany the distributed xv6 files.
