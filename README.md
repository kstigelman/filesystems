# Filesystems Lab
________________________
A simple, UNIX-like file system lab for CSCI-380 Operating Systems.
The assignment was to complete the file system in a C file (fs.c).
All other code was provided by the professor (create_fs.c, runner.c, Makefile).

File system is on a disk (represented as a file) that is 128 KB in size.
- Supports up to 16 files
- Maximum filesize is 8 blocks. Block size is 1 KB
- Files have unique names of 15 characters.

The first 1 KB block is the super block, containing the free block list and inodes.
Remaining 127 KB stores data.

Executing the command
  "runner.exe < input.txt"
in a shell will redirect the commands in input.txt to the program, and read/write the disk accordingly
