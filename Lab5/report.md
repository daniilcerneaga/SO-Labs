# Laboratory Work No. 5 — The File System with Your Own Hands

**Student:** Daniil Cerneaga, FAF-243  
**Supervisors:** Marius Dumitrescu; Reițman Patricia  
**Course:** Operating Systems, FCIM, Technical University of Moldova  
**Date:** 12 November 2026

## Purpose and environment

The experiments connect filenames to inodes, examine hard and symbolic links, distinguish logical file size from allocated space, and test file/directory permissions. Linux also exposes disks and process state through named interfaces, which are inspected in the final part.

The work ran as **deventeex, UID 1000**, in Ubuntu 26.04.1 under WSL2, kernel `6.18.54.1-microsoft-standard-WSL2`. It did not run as root: otherwise privileged access could invalidate the permission observations. A fresh directory under the account's home kept the test files on the Linux ext4 filesystem. The recorded umask was `0022`. Resource measurements refer to the Linux guest, not the laptop's complete Windows-visible hardware.

## Part 1. The tree and file types

The survey ran `ls /`, `ls -ld /etc /etc/passwd /bin /dev/null /tmp`, a short `/dev` listing, `file`, and `du`. The focused listings and disk-usage results were:

```text
$ ls -ld /etc /etc/passwd /bin /dev/null /tmp
lrwxrwxrwx  1 root root    7 Apr 20  2026 /bin -> usr/bin
crw-rw-rw-  1 root root 1, 3 Nov 12 15:30 /dev/null
drwxr-xr-x 95 root root 4096 Nov 12 15:30 /etc
-rw-r--r--  1 root root 1386 Sep 22 10:06 /etc/passwd
drwxrwxrwt  8 root root  160 Nov 12 15:30 /tmp

$ file /usr/bin/ls /etc/passwd /bin
/usr/bin/ls: symbolic link to ../lib/cargo/bin/coreutils/ls
/etc/passwd: ASCII text
/bin:        symbolic link to usr/bin

$ du -sh /var/log 2>/dev/null; du -sh /usr 2>/dev/null
181M	/var/log
5.8G	/usr

$ ls -l /dev/null /dev/sdd
crw-rw-rw- 1 root root 1,  3 Nov 12 15:30 /dev/null
brw-rw---- 1 root disk 8, 48 Nov 12 15:30 /dev/sdd
```

*Listing 1.1 — Path types, executable indirection, space usage and device entries.*

### Observe 1 — Types of the five paths

| Path | First character | Meaning |
| --- | --- | --- |
| `/etc` | d | Directory containing system configuration. |
| `/etc/passwd` | - | Regular text file containing account metadata. |
| `/bin` | l | Symbolic link to usr/bin. |
| `/dev/null` | c | Character device. |
| `/tmp` | d | Directory; the final t marks its sticky bit. |

*Table 1.1 — File types read from ls -ld.*

### Observe 2 — What /bin is on this system

`/bin` is a symbolic link to the relative target `usr/bin`, which resolves to `/usr/bin` from the root directory. A further system-specific detail is that `/usr/bin/ls` itself is a link to `../lib/cargo/bin/coreutils/ls`; the `file` result therefore reports a link rather than directly describing an ELF binary.

### Observe 3 — Character and block devices

`/dev/null` starts with `c` and has device numbers 1,3. `/dev/sdd` starts with `b` and has numbers 8,48. The first is a stream-style device interface; the second is the virtual block device holding the guest's root filesystem. Device numbers identify the kernel interface instead of reporting an ordinary file length.

### Observe 4 — Comparing /var/log and /usr

`du` reported **181M** for `/var/log` and **5.8G** for `/usr`. With these rounded binary-unit displays, `/usr` occupied about 33 times as much space, approximately 5.6 GiB more. Permission diagnostics were suppressed for this size survey, so these are the totals accessible to the ordinary account, not a claim of privileged inspection of every file.


## Part 2. Inodes, links and allocated space

The initial file contained `hello` plus a newline. `stat` and `ls -li` associated it with inode **61740**, link count 1 and length 6. The following excerpts show the same inode after adding and removing names:

```text
$ echo hello > notes.txt; stat notes.txt; ls -li notes.txt
  File: notes.txt
  size: 6         	Blocks: 8          IO Block: 4096   regular file
Device: 8,48	Inode: 61740       Links: 1
Access: (0644/-rw-r--r--)  Uid: ( 1000/deventeex)   Gid: ( 1000/deventeex)
Access: 2026-11-12 15:31:01.284356885 +0200
Modify: 2026-11-12 15:31:01.286769884 +0200
Change: 2026-11-12 15:31:01.286769884 +0200
 Birth: 2026-11-12 15:31:01.284356885 +0200
61740 -rw-r--r-- 1 deventeex deventeex 6 Nov 12 15:31 notes.txt

$ ln notes.txt hard.txt; ls -li notes.txt hard.txt
61740 -rw-r--r-- 2 deventeex deventeex 6 Nov 12 15:31 hard.txt
61740 -rw-r--r-- 2 deventeex deventeex 6 Nov 12 15:31 notes.txt

$ rm notes.txt; cat hard.txt; ls -li hard.txt
hello
61740 -rw-r--r-- 1 deventeex deventeex 6 Nov 12 15:31 hard.txt
```

*Listing 2.1 — The inode remains the same across hard-link creation and unlink.*

### Observe 1 — Inode and link count

`Inode: 61740` identifies the file on this filesystem, and `Links:` counts its directory entries. `ls -i` prints the same inode number. It is the inode, not the first filename used, that is the shared object behind the hard links.

### Observe 2 — Removing the original name

After `ln notes.txt hard.txt`, both rows showed inode 61740 and link count 2. Removing `notes.txt` left `hard.txt` readable as `hello` and reduced the count to 1. `rm` unlinks a directory entry; it does not privilege or erase an “original” name while another link still exists. Releasing file data also depends on open references, as the additional experiment below demonstrates.

### Observe 3 — The broken symbolic link

```text
$ ln -s hard.txt soft.txt; ls -li hard.txt soft.txt; cat soft.txt
61740 -rw-r--r-- 1 deventeex deventeex 6 Nov 12 15:31 hard.txt
61741 lrwxrwxrwx 1 deventeex deventeex 8 Nov 12 15:31 soft.txt -> hard.txt
hello

$ rm hard.txt; cat soft.txt; ls -l soft.txt
lrwxrwxrwx 1 deventeex deventeex 8 Nov 12 15:31 soft.txt -> hard.txt
cat: soft.txt: No such file or directory
```

*Listing 2.2 — A symbolic link continues to exist after its target name is removed.*

`soft.txt` was a separate inode, **61741**, with type `l`. Its stored target is the pathname `hard.txt`, exactly **8 bytes** long. Once `hard.txt` was removed, that stored pathname could no longer be resolved, so `cat soft.txt` failed even though the symlink's own directory entry still existed. Its content is a name to follow, rather than another direct reference to inode 61740.

### Observe 4 — Directory links

Before running the experiment, the prediction was 2 links for a new ext4 directory and 4 after adding two subdirectories. `mkdir d; ls -ld d` and `mkdir d/x d/y; ls -ld d` confirmed those values. One reference comes from the parent's entry naming d and one from d's own `.`; each child contributes a `..` reference to d. This is the Linux/ext4 convention observed here, not an assumption about every filesystem.

### Observe 5 — Size is not allocation

```text
$ echo x > one.txt; ls -l one.txt; du -h one.txt
-rw-r--r-- 1 deventeex deventeex 2 Nov 12 15:31 one.txt
4.0K	one.txt

$ truncate -s 1G big.img; ls -lh big.img; du -h big.img; rm big.img
-rw-r--r-- 1 deventeex deventeex 1.0G Nov 12 15:31 big.img
0	big.img
```

*Listing 2.3 — A small allocated file and a large sparse file.*

`one.txt` contained **2 bytes**, but used one 4096-byte filesystem block, displayed as 4.0K. The earlier `stat` result's Blocks count uses 512-byte accounting units, so 8 such units also represent 4096 bytes. `truncate -s 1G big.img` established a logical size of **1 GiB** without writing data into its holes; `du` consequently showed **0** allocated data space. Metadata still exists, so this does not mean the filesystem stores literally no information about the file.


## Part 3. Permissions

### Observe 1 — Numeric modes and uses

| Mode | Permission bits | Example use |
| --- | --- | --- |
| 600 | rw------- | A private data file readable/writable only by its owner. |
| 644 | rw-r--r-- | A document others may read but only the owner may edit. |
| 755 | rwxr-xr-x | A directory others can list/traverse, or a public executable. |
| 700 | rwx------ | A private directory or executable restricted to its owner. |

*Table 3.1 — Nine access bits; the separate file-type character is not included.*

### Observe 2 — Ownership does not imply read access

```text
$ echo secret > s.txt; chmod 600 s.txt; ls -l s.txt
-rw------- 1 deventeex deventeex 7 Nov 12 15:31 s.txt

$ chmod 000 s.txt; cat s.txt
cat: s.txt: Permission denied

$ chmod u+r s.txt; cat s.txt
secret
```

*Listing 3.1 — Denied reading at mode 000 and restored owner-read permission.*

The owner still undergoes the access-bit check. With mode 000 the ordinary account had no read permission, and `cat` reported Permission denied. Ownership allowed changing the mode with `chmod u+r`; it did not itself bypass the check. Privileged capabilities can be an exception, which is why the experiment used UID 1000.

### Observe 3 — umask and defaults

The umask `0022` removed group and other write bits from the usual creation masks. A new ordinary file therefore received `0666 & ~0022 = 0644`; a new directory received `0777 & ~0022 = 0755`. The observed `new.txt` row was `-rw-r--r--`, while `newdir` was `drwxr-xr-x`. New regular files do not gain execute bits from the usual file-creation request.

### Observe 4 — Read and search permission on a directory

```text
$ mkdir box; echo hi > box/f; chmod u=r,go= box; ls box; cat box/f
f
cat: box/f: Permission denied

$ chmod u=x,go= box; ls box; cat box/f
hi
ls: cannot open directory 'box': Permission denied

$ chmod 755 box
```

*Listing 3.2 — Read-only versus execute-only access to box.*

The prediction was that `u=r` would let `ls box` reveal `f` but prevent opening `box/f`, whereas `u=x` would prevent listing names but allow reading that known pathname. Both predictions were confirmed. Directory read permission permits enumerating names; execute/search permission permits resolving names through that directory. Reading the file also requires permission on the file itself. The final `chmod 755 box` restored normal access.

### Observe 5 — setuid and sticky bits

```text
$ ls -l /usr/bin/passwd; ls -ld /tmp
-rwsr-xr-x 1 root root 93640 Feb  2  2026 /usr/bin/passwd
drwxrwxrwt 8 root root 180 Nov 12 15:30 /tmp

$ cat /etc/shadow; ls -l /etc/shadow
-rw-r----- 1 root shadow 781 Sep 22 10:06 /etc/shadow
cat: /etc/shadow: Permission denied
```

*Listing 3.3 — passwd setuid, /tmp sticky bit and shadow protection.*

The `s` in `/usr/bin/passwd`'s owner-execute position means setuid plus execute. When honoured, it makes the program run with its file owner's effective UID, here root, so password management can access protected authentication data while enforcing its own checks. The `t` in `/tmp` is the sticky bit: write permission on the shared directory does not let an ordinary user remove another user's entries. The file `/etc/shadow` remained unreadable to the tested account.


## Part 4. Disks and the kernel as files

### Observe 1 — Disk layout

`lsblk` showed whole virtual devices rather than partition rows for the root volume. The Linux-visible layout relevant to this experiment was:

```text
/dev/sda   722.9 MiB, read-only virtual image
/dev/sdb    97 MiB, read-only, kernel headers/tools mounts
/dev/sdc     1 GiB, swap
/dev/sdd     1 TiB, virtual block device
  +-- ext4 mounted at /
  +-- also visible at /mnt/wslg/distro
```

*Listing 4.1 — Diagram of the block devices reported by lsblk.*

The root mount was `/dev/sdd` with type **ext4**. Windows drive letters appeared separately under `/mnt/c` and `/mnt/d`; the virtual 1 TiB capacity does not describe the laptop's physical partition layout or reserved physical storage.

### Observe 2 — Root filesystem occupancy

```text
$ findmnt -n -o SOURCE,FSTYPE,TARGET /; df -h /
/dev/sdd ext4 /
Filesystem      Size  Used Avail Use% Mounted on
/dev/sdd       1007G  3.8G  952G   1% /
```

*Listing 4.2 — Root mount and usage.*

The filesystem displayed **1007G total, 3.8G used, 952G available, 1% used**. The percentage is rounded, and available space may differ from total minus used because filesystem accounting can reserve space. These are the values from this run.

### Observe 3 — CPU and RAM visible through /proc

```text
$ grep -c processor /proc/cpuinfo
8

$ head -3 /proc/meminfo
MemTotal:        3946180 kB
MemFree:         2492360 kB
MemAvailable:    3336212 kB
```

*Listing 4.3 — Linux-visible processors and memory.*

The guest reported **8 logical processors**, not necessarily eight physical cores, and `MemTotal` was **3,946,180 kB**, approximately **3.76 GiB**. The distinction matters under WSL2: the guest view is not a measurement of all RAM installed in the notebook. Free and available RAM were 2,492,360 and 3,336,212 kB in this snapshot.

### Observe 4 — Who /proc/self describes

```text
$ head -5 /proc/self/status
Name:	head
Umask:	0022
State:	R (running)
Tgid:	1554
Ngid:	0

$ ls -l /proc/self/fd
total 0
lr-x------ 1 deventeex deventeex 64 Nov 12 15:31 0 -> pipe:[1984]
l-wx------ 1 deventeex deventeex 64 Nov 12 15:31 1 -> pipe:[8405]
l-wx------ 1 deventeex deventeex 64 Nov 12 15:31 2 -> pipe:[8406]
lr-x------ 1 deventeex deventeex 64 Nov 12 15:31 3 -> /proc/1555/fd
```

*Listing 4.4 — Process-relative kernel information.*

`head -5 /proc/self/status` showed **Name: head**, because self resolves for the process opening that file. It did not describe Bash or a fixed process chosen in advance. Likewise, `ls -l /proc/self/fd` described ls's own descriptors. In the recorded command session, descriptors 0–2 pointed to pipes used to collect command output; descriptor 3 pointed to the directory being enumerated.

## Additional experiments

### File signature independent of extension

```text
$ gzip -c /etc/passwd > compressed.txt; file compressed.txt; xxd compressed.txt | head -1
compressed.txt: gzip compressed data, was "passwd", last modified: Tue Sep 22 07:06:27 2026, from Unix, original size modulo 2^32 1386
00000000: 1f8b 0808 f328 b26a 0003 7061 7373 7764  .....(.j..passwd
```

*Listing 5.1 — A gzip stream saved with a .txt filename.*

The leading bytes **1f 8b** identify gzip data. `file` recognised the compressed stream despite the `.txt` extension; a filename suffix does not determine a file's content format.

### A removed file kept open

```text
$ echo kept-open > bigfile; exec 3< bigfile; rm bigfile; ls -l /proc/$$/fd/3; cat <&3; exec 3<&-
lr-x------ 1 deventeex deventeex 64 Nov 12 15:31 /proc/1563/fd/3 -> /home/deventeex/os-lab5-asmwqog7/bigfile (deleted)
kept-open
```

*Listing 5.2 — An open descriptor still reaches a removed file.*

After unlinking `bigfile`, the descriptor link ended with `(deleted)`, and `cat <&3` still read `kept-open`. The final `exec 3<&-` closed the remaining descriptor. With no directory links and no remaining open reference, the inode and data can be reclaimed; removing a name and ending access to its contents are separate events.

## Conclusion

The experiments showed that names, inode identity, access rights and data allocation describe different properties of a file. A hard link preserved access after one name was removed, a symbolic link retained only an unresolved path, and a large logical size did not require allocated data blocks. The permission and /proc experiments also made the execution context visible: effective privileges determine access, and self refers to the program making the current request.

### Sources

Course handout: “Lab 5 — The File System with Your Own Hands”. The observations were checked against the actual `ls`, `stat`, `file`, `du`, permission and `/proc` outputs in the Linux session.
