/*
  Filename   : fs.c
  Author     : Kyler Stigelman
  Course     : CSCI 380-01
  Assignment : Filesystems
  Description: A simple UNIX-like file system that resides on the disk.
               The disk is 128KB and can support 16 files. A file is
               8 blocks in size, and each block is 1KB.
 */

/*
 *******************************************************************************
 * INCLUDE DIRECTIVES
 *******************************************************************************
 */

#include "fs.h"
// For open, close
#include <fcntl.h>
// For read, write, lseek
#include <unistd.h>
// For printf
#include <stdio.h>
// For exit
#include <stdlib.h>
// For strcmp, strtok
#include <string.h>

/*
 *******************************************************************************
 * CONSTANTS
 *******************************************************************************
 */

#define FS_NUM_BLOCKS    128
#define FS_MAX_FILE_SIZE 8
#define FS_MAX_FILENAME  16
#define FS_MAX_FILES     16
#define FS_BLOCK_SIZE    1024


/*
 *******************************************************************************
 * STRUCTS
 *******************************************************************************
 */

//Struct for the filesystem file.
struct fs_t
{
  int fd;
};

//Struct containing the features of an inode.
typedef struct
{
  char name[16];
  int size;
  int blockPointers[8];
  int used;
} Inode;

// open the file with the above name
void
fs_open (struct fs_t *fs, char diskName[16])
{
  // this file will act as the "disk" for your file system
  fs->fd = open (diskName, O_RDWR, 0600);
  if (fs->fd == -1) {
    printf ("Error: Could not open disk %s", diskName);
    exit (0);
  }
}

// close and clean up all associated things
void
fs_close (struct fs_t *fs)
{
  // this file will act as the "disk" for your file system
  if (close (fs->fd) == 0) {
    fs->fd = 0;
    return;
  }
  printf ("Error: Failed to close file with descriptor %d\n", fs->fd);
  exit (0);
}

// create a file with this name and this size
void
fs_create (struct fs_t *fs, char name[16], int size)
{
  // high level pseudo code for creating a new file
  // Step 1: check to see if we have sufficient free space on disk by
  // reading in the free block list. To do this:
  // - move the file pointer to the start of the disk file.
  // - Read the first 128 bytes (the free/in-use block information)
  // - Scan the list to make sure you have sufficient free blocks to
  //   allocate a new file of this size
  if (size > FS_MAX_FILE_SIZE) {
    printf ("Error: Requested file size is too big!");
    return;
  }

  char block_buffer[FS_NUM_BLOCKS];
  lseek (fs->fd, 0, SEEK_SET);
  read (fs->fd, block_buffer, FS_NUM_BLOCKS);
  int count = 0;
  for (int i = 0; i < FS_NUM_BLOCKS; ++i) {
    if (block_buffer[i] == 0)
      ++count;
    if (count == size)
      break;
  }
  if (count < size) {
    printf ("Error: Not enough space on disk\n");
    return;
  }

 // Step 2: we look for a free inode on disk
  // - Read in a inode
  // - check the "used" field to see if it is free
  // - If not, repeat the above two steps until you find a free inode
  // - Set the "used" field to 1
  // - Copy the filename to the "name" field
  // - Copy the file size (in units of blocks) to the "size" field
  // Step 3: Allocate data blocks to the file
  // - Scan the block list that you read in Step 1 for a free block
  // - Once you find a free block, mark it as in-use (Set it to 1)
  // - Set the blockPointer[i] field in the inode to this block number.
  // - repeat until you allocated "size" blocks
  // Step 4: Write out the inode and free block list to disk
  // - Move the file pointer to the start of the file
  // - Write out the 128 byte free block list
  // - Move the file pointer to the position on disk where this inode was stored
  // - Write out the inode
  int nextIndex = 0;
  for (int i = 0; i < FS_MAX_FILES; ++i) {
    Inode inode;
    readInode (fs->fd, &inode, i);
    if (strncmp (name, inode.name, 16) == 0) {
      printf ("Error: File already exists!");
      return;
    }
    if (inode.used == 0) {
      inode.used = 1;
      strncpy (inode.name, name, FS_MAX_FILENAME);
      inode.size = size;
      for (int j = 0; j < FS_NUM_BLOCKS; ++j) {
        if (block_buffer[j] == 0)
        {
          block_buffer[j] = 1;
          inode.blockPointers[nextIndex] = j;
          ++nextIndex;
          if (nextIndex == size)
            break;
         
        }
      }
      writeInode (fs->fd, &inode, i);
      printInode (&inode);
      break;
    }
    //If we get to this point, 16 files have already been made.
    if (i == FS_MAX_FILES - 1) {
      printf ("Error: Too many files already created!\n");
    }
  }
  //Write free block list
  lseek (fs->fd, 0, SEEK_SET);
  write (fs->fd, block_buffer, FS_NUM_BLOCKS);
}

// Delete the file with this name
void
fs_delete (struct fs_t *fs, char name[16])
{
  // Step 1: Locate the inode for this file
  //   - Move the file pointer to the 1st inode (129th byte)
  //   - Read in a inode
  //   - If the iinode is free, repeat above step.
  //   - If the iinode is in use, check if the "name" field in the
  //     inode matches the file we want to delete. IF not, read the next
  //     inode and repeat
  Inode inode;
  lseek (fs->fd, FS_NUM_BLOCKS, SEEK_SET);
  for (int i = 0; i < FS_MAX_FILES; ++i)
  {
    readInode (fs->fd, &inode, i);
    if (inode.used == 1 && strcmp (inode.name, name) == 0)
    {
      // Step 2: free blocks of the file being deleted
      // Read in the 128 byte free block list (move file pointer to start
      //   of the disk and read in 128 bytes)
      lseek (fs->fd, 0, SEEK_SET);
      char block_list[FS_NUM_BLOCKS];
      read (fs->fd, block_list, FS_NUM_BLOCKS);

      // Free each block listed in the blockPointer fields
      for (int j = 0; j < inode.size; ++j)
        block_list[inode.blockPointers[j]] = 0;

      // Step 3: mark inode as free
      // Set the "used" field to 0.
      inode.used = 0;

      // Step 4: Write out the inode and free block list to disk
      // Move the file pointer to the start of the file
      // Write out the 128 byte free block list
      lseek (fs->fd, 0, SEEK_SET);
      write (fs->fd, block_list, FS_NUM_BLOCKS);
      
      // Write out the inode to disk
      writeInode (fs->fd, &inode, i);
      return;
    }
  }
  printf ("Error: File not found\n");
}

// List names of all files on disk
void
fs_ls (struct fs_t *fs)
{
  // Step 1: read in each inode and print!
  // Move file pointer to the position of the 1st inode (129th byte)
  // for each inode:
  //   read it in
  //   if the inode is in-use
  //     print the "name" and "size" fields from the inode
  // end for
  lseek (fs->fd, FS_NUM_BLOCKS, SEEK_SET);
  Inode inode;
  for (int i = 0; i < FS_MAX_FILES; ++i) {
    readInode (fs->fd, &inode, i);
    if (inode.used == 1)
      printf ("%16s %6dB\n", inode.name, inode.size * 1024);
  }
}

// read this block from this file
void
fs_read (struct fs_t *fs, char name[16], int blockNum, char buf[1024])
{
  // Step 1: locate the inode for this file
  //   - Move file pointer to the position of the 1st inode (129th byte)
  //   - Read in a inode
  //   - If the inode is in use, compare the "name" field with the above file
  //   - If the file names don't match, repeat
  lseek (fs->fd, FS_NUM_BLOCKS, SEEK_SET);
  Inode inode;
  for (int i = 0; i < FS_MAX_FILES; ++i)
  {
    readInode (fs->fd, &inode, i);
    if (inode.used == 1 && strcmp (inode.name, name) == 0)
    {
      // Step 2: Read in the specified block
      // Check that blockNum < inode.size, else flag an error
      
      if (blockNum >= inode.size) {
        printf ("Error: Block index too large");
        return;
      }
      // Get the disk address of the specified block
      // move the file pointer to the block location
      // Read in the block! => Read in 1024 bytes from this location into the buffer
      // "buf"
      lseek (fs->fd, FS_BLOCK_SIZE * inode.blockPointers[blockNum], SEEK_SET);
      read (fs->fd, buf, FS_BLOCK_SIZE);
      return;
    }
  }
  printf ("Error: File not found\n");
}

// write this block to this file
void
fs_write (struct fs_t *fs, char name[16], int blockNum, char buf[1024])
{
  // Step 1: locate the inode for this file
  // Move file pointer to the position of the 1st inode (129th byte)
  // Read in a inode
  // If the inode is in use, compare the "name" field with the above file
  // If the file names don't match, repeat
  lseek (fs->fd, FS_NUM_BLOCKS, SEEK_SET);
  Inode inode;
  for (int i = 0; i < FS_MAX_FILES; ++i)
  {
    readInode (fs->fd, &inode, i);
    if (inode.used == 1 && strcmp (inode.name, name) == 0)
    {
      // Step 2: Write to the specified block
      // Check that blockNum < inode.size, else flag an error
      if (blockNum >= inode.size) {
        printf ("Error: Block index too large");
        return;
      }
      // Get the disk address of the specified block
      // move the file pointer to the block location
      // Write the block! => Write 1024 bytes from the buffer "buf" to this location
      lseek (fs->fd, FS_BLOCK_SIZE * inode.blockPointers[blockNum], SEEK_SET);
      write (fs->fd, buf, FS_BLOCK_SIZE);
      return;
    }
  }
  printf ("Error: File not found\n");
}

// REPL entry point
void
fs_repl ()
{
  char name[16];
  fgets (name, sizeof (name), stdin);
  strtok (name, "\n");
  struct fs_t fs;
  fs_open (&fs, name);

  char command[32];
  while (fgets (command, sizeof (command), stdin))
  {
    strtok (command, "\n");
    int size;
    char str[16];
    if (sscanf (command, "C %15s %d", str, &size) == 2) {
      printf ("Creating file: %15s %d\n", str, size); 
      fs_create (&fs, str, size);
    }
    else if (sscanf (command, "D %15s", str) == 1) {
      printf ("Deleting file: %s\n", str);
      fs_delete (&fs, str);
    }
    else if (strcmp (command, "L") == 0) {
      printf ("Listing files:\n");
      fs_ls (&fs);
    }
    else if (sscanf (command, "R %15s %d", str, &size) == 2) {
      printf ("Reading from file: %s %d\n", str, size);

    }
    else if (sscanf (command, "W %7s %d", str, &size) == 2) {
      printf ("Writing to file: %s %d\n", str, size);
    }
    else
      printf ("Unknown command\n"); 
  }
  fs_close (&fs);
}

//Writes an inode to the file. lseek to the inode index, then write.
void 
writeInode (int fd, Inode* inode, int inodeNum) {
  lseek (fd, FS_NUM_BLOCKS + (inodeNum * sizeof (Inode)), SEEK_SET);
  write (fd, inode, sizeof (Inode));
}

//Reads an inode from file. lseek to the inode index, then write.
void
readInode (int fd, Inode* inode, int inodeNum)
{
  lseek (fd, FS_NUM_BLOCKS + (inodeNum * sizeof (Inode)), SEEK_SET);
  read (fd, inode, sizeof (Inode));
}

//Print out the inode
void
printInode (const Inode* inode) 
{
  printf ("%15s: %d blocks, Block pointers: ",  inode->name, inode->size);
  for (int i = 0; i < FS_MAX_FILE_SIZE; ++i)
    printf ("%d ", inode->blockPointers[i]);
  printf ("Used: %d\n", inode->used);
}