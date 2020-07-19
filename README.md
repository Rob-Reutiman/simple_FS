# Operating Systems Project 06: Simple File System

For this project, I created a simplified version of the Unix File System.

This project is made up of three main components:

Shell: The first component is a simple shell application that allows the user to perform operations on the SimpleFS such as printing debugging information about the file system, formatting a new file system, mounting a file system, creating files, and copying data in or out of the file system. To do this, it translates these user commands into file system operations.

File System: The second component takes the operations specified by the user through the shell and performs them on the SimpleFS disk image. This component is charged with organizing the on-disk data structures and performing all the bookkeeping necessary to allow for persistent storage of data. To store the data, it interacts with the disk emulator via functions such as disk_read and disk_write, which allow the file system read and write to the disk image in 4096 byte blocks.

Disk Emulator: The third component emulates a disk by creating a disk image (created by dividing a normal file into 4096 byte blocks) and only allowing the File System to read and write in terms of blocks. This emulator persistently stores data to the disk image using the normal open, read, and write system calls.
