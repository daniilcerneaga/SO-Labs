# Laboratory Work No. 6 — Inside the xv6 File System

**Student:** Daniil Cerneaga, FAF-243  
**Supervisors:** Marius Dumitrescu; Reițman Patricia  
**Course:** Operating Systems, FCIM, Technical University of Moldova  
**Date:** 12 November 2026

## Purpose and environment

This lab examines the concrete structures behind xv6 filenames: directory entries, inode metadata, hard links, fixed block-addressing limits and recursive tree traversal. Two utilities, `mystat` and `find`, expose these ideas through ordinary xv6 system calls.

The code was compiled from course-pinned revision `06aad25c735fd3159bdfae5680be4eab7b1668b2` using RISC-V GCC 15.2.0, then booted with QEMU 10.2.1, one hart and 128 MiB of RAM. Linux-side build commands ran in Ubuntu 26.04.1 under WSL2. A clean filesystem image separated the required find example from the earlier link experiments; later test images also recreated their own test trees rather than assuming those files survived an image rebuild.

## Part 1. Reading directory metadata

The root listing included `README` and the supplied programs, each with a numeric type, inode and size. A new directory a was then created and inspected.

```text
$ mystat a
a: directory, inode 26, links 1, size 32 bytes
$ mystat README . console
README: file, inode 2, links 1, size 2441 bytes
.: directory, inode 1, links 2, size 1024 bytes
console: device, inode 25, links 1, size 0 bytes
```

*Listing 1.1 — An empty directory, a regular file, the root and a device.*

### Observe 1 — Type numbers

`kernel/stat.h` defines **T_DIR = 1**, **T_FILE = 2**, and **T_DEVICE = 3**. Thus the listing's type 1 identifies a directory, type 2 a regular file, and type 3 the console device. These numbers describe this xv6 ABI, not the character codes printed by Linux ls.

### Observe 2 — Root inode and its two dot entries

The root inode is **1**, also defined as ROOTINO in `kernel/fs.h`. At the root, both `.` and `..` refer to that inode because the root has no different parent to ascend to. The reported root link count here was **2**, since a child directory a had already been created; this is a different observation from an example that stats the untouched root first.

### Observe 3 — Empty-directory size

The new a had size **32 bytes** and contained `.` and `..`. `struct dirent` stores a 2-byte inode number and a DIRSIZ-sized name field of 14 bytes, so each entry occupies **16 bytes**, and two entries occupy 32. The observed xv6 link count of a was 1; Linux/ext4's new-directory count of 2 from Lab 5 should not be assumed for xv6's accounting.


## Part 2. Implementing mystat and testing hard links

`mystat` calls the stat library function for every pathname, converts T_DIR/T_FILE/T_DEVICE into readable words, and prints inode, link count and byte size. Size is printed with `%lu` because `struct stat.size` is a 64-bit unsigned value in this revision. A failed stat writes a diagnostic to descriptor 2 and continues to subsequent arguments; the final status is 1 if any argument failed. With no path, it prints usage instead of silently doing nothing.

### Observe 1 — Adding and removing a hard link

```text
$ ln README r2
$ mystat README r2
README: file, inode 2, links 2, size 2441 bytes
r2: file, inode 2, links 2, size 2441 bytes
$ rm README
$ mystat r2
r2: file, inode 2, links 1, size 2441 bytes
$ cat r2 | wc
48 336 2441
```

*Listing 2.1 — Hard-link counts change while inode and contents remain the same.*

After ln, both names identified **inode 2**, link count **2**, and size **2441 bytes**. After unlinking README, r2 retained inode 2 and the same size, but its count was 1. The remaining name still yielded **48 lines, 336 words, 2441 bytes**. Unlinking a name did not copy, truncate or rename the underlying file.

### Observe 2 — Link-count decrement and block reclamation

In the pinned `kernel/sysfile.c`, **line 244** in sys_unlink executes `ip->nlink--`, updates the inode and releases its reference. Another decrement at line 239 concerns the parent directory when a child directory is removed; it is not the regular-file decrement being sought. The unrelated decrement at line 178 is in a different operation, so an unqualified grep result is not enough to identify the correct code.

In `kernel/fs.c`, line **357** checks `ip->ref == 1 && ip->valid && ip->nlink == 0` inside iput. On that last-reference path, line **365** calls `itrunc(ip)` to release the data blocks. Both conditions matter: zero directory links alone does not immediately remove data still referenced by an open file.

### Observe 3 — On-disk inode fields

| Field | Meaning |
| --- | --- |
| `type` | Directory, regular-file or device type. |
| `major`, `minor` | Device numbers for a device inode. |
| `nlink` | Number of filesystem links tracked by xv6. |
| `size` | Logical file size in bytes. |
| `addrs[NDIRECT + 1]` | Twelve direct data-block addresses and one indirect-block address. |

*Table 2.1 — Fields of struct dinode in kernel/fs.h.*

This inode structure contains no UID, GID, access-mode bits or timestamps. The base filesystem has no symbolic-link inode type or symlink-resolution mechanism. The absence of those mechanisms is why the Linux permission and symbolic-link experiments do not translate directly to this xv6 version.


## Part 3. Measuring filesystem limits

The host inspection printed these constants:

```text
5:#define BSIZE   1024
26:#define NDIRECT   12
27:#define NINDIRECT (BSIZE / sizeof(uint))
54:#define DIRSIZ 14
```

*Listing 3.1 — Block and directory-entry limits in the pinned source.*

### Observe 1 — Long names

After `mkdir averyveryverylongname`, ls displayed **averyveryveryl**. DIRSIZ limits the stored name to **14 bytes**; the remaining characters are not stored as part of the entry. The utility copies all 14 bytes to a 15-byte buffer and appends a NUL before strcmp, so a full-length name need not already contain a terminator on disk.

### Observe 2 — Maximum file size

One indirect block stores `1024 / 4 = 256` block numbers. The maximum number of data blocks is therefore `12 + 256 = 268`, yielding **268 × 1024 = 274432 bytes = 268 KiB**. The indirect address block consumes disk space but is not itself part of the file's data capacity.

An independent write test filled a file with 268 blocks, attempted one more byte, and then checked the resulting size:

```text
max-file: size=274432; blocks=268; extra-write=-1
PASS: file-size boundary
exitstatus=0
```

*Listing 3.2 — Actual verification of the calculated maximum file size.*

The additional write returned **-1**, and the size remained **274432**. The temporary test file was removed afterwards. This confirms the source calculation with an executed boundary test rather than assuming that a nominal constant alone describes the observed result.

### Observe 3 — Disk layout

A clean rebuild printed:

```text
nmeta 47 (boot, super, log blocks 31, inode blocks 13, bitmap blocks 1) blocks 1953 total 2000
```

*Listing 3.3 — The filesystem-image layout reported by mkfs.*

The whole disk has **2000 blocks**, or 2,048,000 bytes. Its metadata uses 47 blocks: boot 1, superblock 1, log 31, inode area 13 and bitmap 1. The remaining **1953 blocks** form the data region. They are not all free space: the initial programs, README and directory data already occupy some of that region.


## Part 4. Finding exact names in the tree

The program accepts `find <dir> <name>` and the optional `-n` argument for inode numbers. It tests the last path component with strcmp, skips unused entries with inum 0, skips `.` and `..`, and checks directory reads and stat failures. Directory entries with full 14-byte names are safely terminated before comparison.

The traversal is a depth-first search of the complete hierarchy. Its pending paths are held in an explicit heap stack rather than nesting C function calls. This implements the recursive search while avoiding xv6's small user stack. Only the directory currently being enumerated is open; it is closed before the next pending path is processed, so deep traversal does not consume one descriptor per ancestor. Each pending item is freed after processing.

The required example was created on a fresh filesystem image:

```text
mkdir a
mkdir a/b
echo x > a/b/r2
echo y > a/r2
ln cat r2
```


```text
$ find . r2
./r2
./a/r2
./a/b/r2
$ find a b
a/b
```

*Listing 4.1 — All three matching files and the matching directory.*

The order differs from the handout because the explicit stack visits the most recently queued entry first; the set of matching paths is the required one. A directory itself can be a match, as shown by `a/b`.

### Optional inode-number output

```text
$ find . r2 -n
./r2: inode 3
./a/r2: inode 29
./a/b/r2: inode 28
$ mystat cat r2 a/r2 a/b/r2
cat: file, inode 3, links 2, size 36728 bytes
r2: file, inode 3, links 2, size 36728 bytes
a/r2: file, inode 29, links 1, size 2 bytes
a/b/r2: file, inode 28, links 1, size 2 bytes
```

*Listing 4.2 — Distinguishing the hard link from two independent same-named files.*

Only **./r2** shares cat's inode, **3**. The two nested r2 files are separate files, with inodes **29** and **28**, each 2 bytes long. Finding equal names does not by itself imply equal file identity.

### Additional verification

The checks covered all required paths, a file as the starting path, a trailing slash on the starting directory, a nonexistent name, a removed/free directory slot, and a filename exactly 14 bytes long. Error cases covered no arguments, an invalid option and a nonexistent starting path. A mixed mystat call reported its missing first argument, continued to print README, and exited with status 1.

A further tree contained **20 nested directories** and a file named needle at its leaf. The search found the full path without a trap or an open-file exhaustion error. This test motivated the explicit-stack implementation: a version using nested C calls could cross the guest's stack guard even while closing descriptors correctly. The final submitted version passed the repeated checks, and no test-only program is registered in its Makefile.

## Conclusion

The experiments connected a filename to a fixed-size directory entry, an inode and a bounded set of data blocks. Hard-link tests showed that directory names can change while inode identity and contents stay stable; inode-number output then distinguished a hard link from two unrelated files with the same name. Implementing and testing find also exposed practical resource limits of a small OS, leading to an explicit-stack traversal that succeeded on a 20-level tree.

### Sources

Course handout: “Lab 6 — Inside the xv6 File System”. Structures, constants and line numbers refer to [MIT xv6 at commit 06aad25](https://github.com/mit-pdos/xv6-riscv/tree/06aad25c735fd3159bdfae5680be4eab7b1668b2), especially `kernel/fs.h`, `kernel/stat.h`, `kernel/sysfile.c`, `kernel/fs.c` and the directory-entry example in `user/ls.c`.
