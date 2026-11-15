# Laboratory Work No. 1 — The OS as a Resource Manager

**Student:** Daniil Cerneaga, FAF-243  
**Supervisors:** Marius Dumitrescu; Reițman Patricia  
**Course:** Operating Systems, FCIM, Technical University of Moldova  
**Date:** 12 November 2026

## Purpose and experimental environment

The purpose of this work is to observe four responsibilities of an operating system: maintaining files, scheduling processes, accounting for memory, and exposing devices and storage. The commands and selected outputs below document these responsibilities in a Linux session.

The experiments ran as the ordinary user `deventeex` in Ubuntu 26.04.1 LTS under WSL2. This is the Linux guest environment available on the laptop, rather than a separate course VM. Its memory and virtual disks are the resources visible to Linux; they are not measurements of all the hardware available to Windows. A fresh directory, `/home/deventeex/os-lab1.666xzN`, kept the file experiments separate from earlier work.

Selected output from the environment check:

```text
$ whoami
deventeex
$ uname -a
Linux DESKTOP-7CJE0EE 6.18.54.1-microsoft-standard-WSL2
#1 SMP PREEMPT_DYNAMIC Tue Oct 6 19:13:07 UTC 2026 x86_64 GNU/Linux
$ uptime
14:27:34 up 0 min, 1 user, load average: 0.32, 0.07, 0.02
```

*Listing 0.1 — Linux user, kernel and uptime.*

The long `uname` line is wrapped here for readability. The guest had just started, so the process and memory observations describe this particular startup session, not a long-term average.

### Background and sources

The kernel coordinates access to shared hardware and keeps resource-specific state: file permissions, process identities, memory mappings and device interfaces. The lab uses Linux utilities to inspect this state without changing the kernel. The procedure follows the course handout, “Lab 1 — The OS as a Resource Manager”. Repeating these commands produces different timestamps, PIDs and resource counters, while the permission and process-state interpretations remain the same.

<!-- pagebreak -->

## Part 1. Files and directories

The exploration used `pwd`, `ls -la /`, `ls -la ~`, and `cd /etc && ls | head`. Inside the experiment directory, `mkdir demo` created a subdirectory. Writing `hello operating systems` to `note.txt` produced a 24-byte file: 23 text characters and the newline added by `echo`. The commands `cp note.txt copy.txt`, `mv copy.txt renamed.txt`, and `rm renamed.txt` demonstrated creation of a second file, a name change, and removal of that second file.

The permission experiment used `ls -l note.txt`, `chmod 600 note.txt`, another listing, and finally `chmod 644 note.txt`. An additional `stat` command recorded ownership and permissions without relying on column positions.

```text
$ ls -l
total 8
-rw-r--r-- 1 deventeex deventeex 24 Nov 12 14:27 note.txt
-rw-r--r-- 1 deventeex deventeex 24 Nov 12 14:27 renamed.txt
$ rm renamed.txt
$ chmod 600 note.txt
$ ls -l note.txt
-rw------- 1 deventeex deventeex 24 Nov 12 14:27 note.txt
$ stat -c 'owner=%U group=%G permissions=%A mode=%a size=%s' note.txt
owner=deventeex group=deventeex permissions=-rw------- mode=600 size=24
$ chmod 644 note.txt
$ ls -l note.txt
-rw-r--r-- 1 deventeex deventeex 24 Nov 12 14:27 note.txt
```

*Listing 1.1 — File creation and the change from mode 600 to mode 644.*

### Observe 1 — Owner and group

Both files belonged to user `deventeex` and group `deventeex`. These are the identities recorded by the filesystem for the files created by this account in the test directory.

### Observe 2 — The ten permission characters

After `chmod 600`, the string was `-rw-------`. The initial `-` identifies a regular file. Positions 2–4, `rw-`, give its owner read and write access but no execute access. Positions 5–7 deny all access to the owning group; positions 8–10 do the same for other users. In octal, 6 is read (4) plus write (2), and the other two digits are zero. Restoring mode 644 made the file readable by the group and other users again.

### Observe 3 — Two directories below the root

`/etc` contains system configuration, including the observed `adduser.conf`, `apt`, and `bash.bashrc` entries. `/dev` contains device interfaces such as terminals and `/dev/null`; these entries are not ordinary documents. The permission listing is the most useful output from this part because it shows the same file before and after an access-control change, with its owner and size unchanged.

<!-- pagebreak -->

## Part 2. Processes

The initial survey used `ps aux | head`, `ps aux | wc -l`, and one frame of `top`. For a reproducible text record, `top` ran in batch mode as `top -b -n 1 | head -16`. This provides the task counters without requiring an interactive screen capture.

The first `sleep 300 &` produced PID 486. `jobs -l` showed the shell's background job, while `ps -ef | grep sleep` showed it in the system process table. `kill 486` sent the default termination signal, and the script waited for its child to be reaped. A second background process, PID 489, was inspected through `/proc` and then terminated as well.

```text
$ ps aux | wc -l
61
$ ps -e --no-headers | wc -l
60
$ ps -p 1 -o pid,comm,args
    PID COMMAND         COMMAND
      1 systemd         /sbin/init

Tasks: 60 total, 1 running, 59 sleeping, 0 stopped, 0 zombie
$ jobs -l
[1]+   486 Running                    sleep 300 &
$ cat /proc/489/status | head -20
Name:   sleep
State:  S (sleeping)
Pid:    489
PPid:   338
```

*Listing 2.1 — Process counts, PID 1 and a background sleeping process.*

The excerpts omit unrelated process rows and the remaining status fields.

### Observe 1 — Process number 1

Its PID is 1, and on this guest it is `systemd`, invoked as `/sbin/init`. It manages system services and serves as the guest's init process. The separate WSL `/init` process appeared with PID 2, so the two entries should not be confused.

### Observe 2 — Approximate process count

The environment had about 60 processes. The first pipeline returned 61 lines because `ps aux` includes a heading. The header-free count returned 60, and `top` independently reported 60 tasks in its frame. Commands used to count processes themselves briefly become processes, so these values are snapshots rather than an exact permanent count. This was a lightly loaded, newly started guest, not a carefully stabilized idle benchmark.

### Observe 3 — State of the sleeping process

PID 489 had `State: S (sleeping)`. It was waiting for a timer, rather than repeatedly executing instructions to count time. The `/proc` excerpt is particularly informative: it associates the program name and waiting state with a PID and parent PID, showing that the kernel maintains information about each process separately.

<!-- pagebreak -->

## Part 3. Memory

The commands `free -h` and `cat /proc/meminfo | head -6` measured memory visible to the Linux guest. A separate `sleep 300` process, PID 496, was inspected for resident memory and then terminated. Before reading its status, the collection script checked that the child had actually executed `sleep`, so the measurement did not accidentally describe the short-lived shell child before `exec`.

```text
$ free -h
               total  used   free  shared  buff/cache  available
Mem:           3.8Gi  603Mi  2.7Gi 4.1Mi   620Mi       3.2Gi
Swap:          1.0Gi  0B     1.0Gi

$ cat /proc/meminfo | head -6
MemTotal:        3946176 kB
MemFree:         2834120 kB
MemAvailable:    3328284 kB
Buffers:           51064 kB
Cached:           562732 kB
SwapCached:            0 kB

$ grep -E '^(Name|State|VmRSS|RssAnon|RssFile|RssShmem):' /proc/496/status
Name:   sleep
State:  S (sleeping)
VmRSS:      7888 kB
RssAnon:    1220 kB
RssFile:    6668 kB
RssShmem:      0 kB
```

*Listing 3.1 — Guest memory totals and the resident pages of sleep.*

Spacing in the compact `free` excerpt has been shortened; numeric values have not been changed.

### Observe 1 — Total and free RAM

`MemTotal` was 3,946,176 kB, approximately 3.76 GiB, displayed by `free` as 3.8 GiB. `MemFree` was 2,834,120 kB, approximately 2.70 GiB. Available memory was higher, approximately 3.17 GiB, because the kernel can reclaim some cached memory. Free memory and memory available for starting applications therefore describe different quantities.

### Observe 2 — Swap

Swap is backing storage to which the kernel can move eligible memory pages when managing RAM pressure. This guest had 1,048,576 kB, or 1 GiB, configured, all of it free at the time of the measurement. The block-device listing later identified `/dev/sdc` as the swap device. Configured swap capacity is not additional physical RAM.

### Observe 3 — Resident memory of sleep

`VmRSS` was 7,888 kB, approximately 7.70 MiB. This is larger than one might expect from a program that only waits, since resident memory includes its executable and mapped library pages, not just changing application data. The breakdown helps explain this particular result: 6,668 kB was file-backed, and 1,220 kB was anonymous. RSS alone does not tell how much memory is unique to this process; some file-backed pages can be shared. It also differs from the process's virtual address-space size.

<!-- pagebreak -->

## Part 4. Devices and storage

The storage survey used `df -h`, `lsblk`, `du -sh` on the experiment directory, `ls -l /dev | head`, and `mount | head`. The extra command `findmnt -n -o SOURCE,FSTYPE,TARGET /` selected the root mount directly instead of requiring it to appear in the first few lines of a longer listing.

```text
$ findmnt -n -o SOURCE,FSTYPE,TARGET /
/dev/sdd ext4 /
$ lsblk
NAME MAJ:MIN RM   SIZE RO TYPE MOUNTPOINTS
...
sdc    8:32   0     1G  0 disk [SWAP]
sdd    8:48   0     1T  0 disk /mnt/wslg/distro
                               /
$ ls -l /dev/null /dev/tty
crw-rw-rw- 1 root root 1, 3 Nov 12 14:27 /dev/null
crw-rw-rw- 1 root tty  5, 0 Nov 12 14:27 /dev/tty
```

*Listing 4.1 — Root mount, swap and character devices.*

The ellipsis omits unrelated devices. `df -h` reported the root filesystem as 1007G total, 3.6G used, 953G available, and 1% used. The apparent 1T virtual device capacity does not imply that a physical 1 TB drive is reserved for this guest. The test directory occupied 12K, including allocation for directories and its small remaining file.

### Observe 1 — Root filesystem device

The root filesystem was mounted on `/dev/sdd` with filesystem type `ext4`. Both `findmnt` and the root entry from `mount` identified this device. Windows volumes were mounted separately at `/mnt/c` and `/mnt/d`; they are not the Linux root filesystem.

### Observe 2 — One device entry

`/dev/tty` represents the calling process's controlling terminal. Its leading `c` identifies a character device, and `5, 0` is its major/minor device-number pair rather than a regular-file size. `/dev/null` is another character device: it discards data written to it and returns end-of-file when read.

### Observe 3 — “Everything is a file”

The phrase describes a common interface: programs can open file descriptors and perform I/O on ordinary files, terminals, device interfaces and pipes, while the kernel implements the different behaviour behind each descriptor. It does not mean that every resource is a regular file stored on disk. Comparing `note.txt` with `/dev/tty` made that distinction visible in the first character of their listings.

### Closing synthesis

The OS manages files, processes, memory, and devices/storage. File access was visible through `ls -l` and `chmod`, while `ps` and `/proc/<pid>/status` exposed process management. `free -h` showed memory accounting, and `lsblk` together with `findmnt` connected storage devices to mounted filesystems.
