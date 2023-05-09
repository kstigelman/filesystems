// File: fs.h
// Author: Chad Hogg
// Part of the revised handout for CSCI380 filesystems lab

#ifndef FS_H
#define FS_H

#define FS_MAX_FILENAME  16
#define FS_BLOCK_SIZE    1024

// A structure containing whatever information you think would be good to keep track of
//   about a filesystem.  Will be defined in fs.c.
struct fs_t;

// open the file with this name and initialize the fs
void
fs_open (struct fs_t* fs, char diskName[FS_MAX_FILENAME]);

// close and clean up all associated things
void
fs_close (struct fs_t* fs);

// create a new file with this name and this many blocks.
//  we shall assume that the file size is specified at file
//  creation time and the file does not grow or shrink from
//  this point on
void
fs_create (struct fs_t* fs, char name[FS_MAX_FILENAME], int size);

// delete the file with this name
void
fs_delete (struct fs_t* fs, char name[FS_MAX_FILENAME]);

// read the specified block from this file into the specified buffer
//   blocknum can range from 0 to 7.
void
fs_read (struct fs_t* fs, char name[FS_MAX_FILENAME], int blockNum, char buf[FS_BLOCK_SIZE]);

// write the data in the buffer to the specified block in this file
void
fs_write (struct fs_t* fs, char name[FS_MAX_FILENAME], int blockNum, char buf[FS_BLOCK_SIZE]);

// list the names of all files in the file system and their sizes
//   NOTE: use the format string "%16s %6dB" to print the files and their sizes
void
fs_ls (struct fs_t* fs);

// enter a REPL session that adheres to the interactive requirements below
void
fs_repl ();

#endif//FS_H

