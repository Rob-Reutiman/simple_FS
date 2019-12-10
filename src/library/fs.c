/* fs.c: SimpleFS file system */

#include "sfs/fs.h"
#include "sfs/logging.h"
#include "sfs/utils.h"

#include <stdio.h>
#include <string.h>

/* External Functions */

/**
 * Debug FileSystem by doing the following
 *
 *  1. Read SuperBlock and report its information.
 *
 *  2. Read Inode Table and report information about each Inode.
 *
 * @param       disk        Pointer to Disk structure.
 **/
void    fs_debug(Disk *disk) {
    Block block;

    /* Read SuperBlock */
    if (disk_read(disk, 0, block.data) == DISK_FAILURE) {
        return;
    }

    printf("SuperBlock:\n");
    printf("    %u blocks\n"         , block.super.blocks);
    printf("    %u inode blocks\n"   , block.super.inode_blocks);
    printf("    %u inodes\n"         , block.super.inodes);

    /* Read Inodes */
    if (disk_read(disk, 1, block.data) == DISK_FAILURE) {
        return;
    }

    for(uint32_t i = 0; i < INODES_PER_BLOCK; i++) {
        
        if(block.inodes[i].valid == 1) {
            printf("Inode %u\n", i);
            printf("    size: %u bytes\n", block.inodes[i].size);
            uint32_t direct_num = 0;
            for(uint32_t q = 0; q < POINTERS_PER_INODE; q++) {
                if(block.inodes[i].direct[q]) direct_num++;
            }
            printf("    direct blocks: %u\n", direct_num); 
        }
        
    }

}

/**
 * Format Disk by doing the following:
 *
 *  1. Write SuperBlock (with appropriate magic number, number of blocks,
 *  number of inode blocks, and number of inodes).
 *
 *  2. Clear all remaining blocks.
 *
 * Note: Do not format a mounted Disk!
 *
 * @param       fs      Pointer to FileSystem structure.
 * @param       disk    Pointer to Disk structure.
 * @return      Whether or not all disk operations were successful.
 **/
bool    fs_format(FileSystem *fs, Disk *disk) {

    if(fs->disk == disk) { // might be wrong (don't format mounted disk)
        return false;
    }

    fs->meta_data.magic_number = MAGIC_NUMBER;
    fs->meta_data.blocks = disk->blocks;
    fs->meta_data.inode_blocks = disk->blocks * .9;
    fs->meta_data.inodes = fs->meta_data.inode_blocks * INODES_PER_BLOCK;
 
    for(uint32_t i = 2; i < disk->blocks; i++) {
        if(disk_write(disk, i, 0) == DISK_FAILURE) return false;
    }

    return true;
}

/**
 * Mount specified FileSystem to given Disk by doing the following:
 *
 *  1. Read and check SuperBlock (verify attributes).
 *
 *  2. Verify and record FileSystem disk attribute. 
 *
 *  3. Copy SuperBlock to FileSystem meta data attribute
 *
 *  4. Initialize FileSystem free blocks bitmap.
 *
 * Note: Do not mount a Disk that has already been mounted!
 *
 * @param       fs      Pointer to FileSystem structure.
 * @param       disk    Pointer to Disk structure.
 * @return      Whether or not the mount operation was successful.
 **/
bool    fs_mount(FileSystem *fs, Disk *disk) {

    Block s;
    if(disk_read(disk, 0, s.data) == DISK_FAILURE) { // read data into block
        return false;
    }

    if(s.super.magic_number != MAGIC_NUMBER) { // check if fs
        return false;
    }

    if(fs->disk) {      // no double mount
        return false;
    }

    fs->disk=disk;
    fs->meta_data=s.super;

    bool *bitmap = malloc(disk->blocks * sizeof(bool));
    memset(bitmap, 1, disk->blocks * sizeof(bool));

    for(int i=0; i < s.super.inode_blocks + 1; i++) {
            bitmap[i]=0;
    }

    for(int q=1; q <= s.super.inode_blocks; q++) { // is this necessary

        if (disk_read(disk, q, s.data) == DISK_FAILURE) {
            return false;
        }

        for(uint32_t i = 0; i < INODES_PER_BLOCK; i++) {
            if(s.inodes[i].valid == 1) {
                bitmap[s.super.inode_blocks + 1 + i + ((q-1)*INODES_PER_BLOCK)]=0;
            }
        }
        // very close

    }

    fs->free_blocks=bitmap;

    return true;
}

/**
 * Unmount FileSystem from internal Disk by doing the following:
 *
 *  1. Set FileSystem disk attribute.
 *
 *  2. Release free blocks bitmap.
 *
 * @param       fs      Pointer to FileSystem structure.
 **/
void    fs_unmount(FileSystem *fs) {

    fs->disk=NULL;
    free(fs->free_blocks);

}

/**
 * Allocate an Inode in the FileSystem Inode table by doing the following:
 *
 *  1. Search Inode table for free inode.
 *
 *  2. Reserve free inode in Inode table.
 *
 * Note: Be sure to record updates to Inode table to Disk.
 *
 * @param       fs      Pointer to FileSystem structure.
 * @return      Inode number of allocated Inode.
 **/
ssize_t fs_create(FileSystem *fs) {



    return -1;
}

/**
 * Remove Inode and associated data from FileSystem by doing the following:
 *
 *  1. Load and check status of Inode.
 *
 *  2. Release any direct blocks.
 *
 *  3. Release any indirect blocks.
 *
 *  4. Mark Inode as free in Inode table.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to remove.
 * @return      Whether or not removing the specified Inode was successful.
 **/
bool    fs_remove(FileSystem *fs, size_t inode_number) {



    return false;
}

/**
 * Return size of specified Inode.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to remove.
 * @return      Size of specified Inode (-1 if does not exist).
 **/
ssize_t fs_stat(FileSystem *fs, size_t inode_number) {



    return -1;
}

/**
 * Read from the specified Inode into the data buffer exactly length bytes
 * beginning from the specified offset by doing the following:
 *
 *  1. Load Inode information.
 *
 *  2. Continuously read blocks and copy data to buffer.
 *
 *  Note: Data is read from direct blocks first, and then from indirect blocks.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to read data from.
 * @param       data            Buffer to copy data to.
 * @param       length          Number of bytes to read.
 * @param       offset          Byte offset from which to begin reading.
 * @return      Number of bytes read (-1 on error).
 **/
ssize_t fs_read(FileSystem *fs, size_t inode_number, char *data, size_t length, size_t offset) {
    

    
    return -1;
}

/**
 * Write to the specified Inode from the data buffer exactly length bytes
 * beginning from the specified offset by doing the following:
 *
 *  1. Load Inode information.
 *
 *  2. Continuously copy data from buffer to blocks.
 *
 *  Note: Data is read from direct blocks first, and then from indirect blocks.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to write data to.
 * @param       data            Buffer with data to copy
 * @param       length          Number of bytes to write.
 * @param       offset          Byte offset from which to begin writing.
 * @return      Number of bytes read (-1 on error).
 **/
ssize_t fs_write(FileSystem *fs, size_t inode_number, char *data, size_t length, size_t offset) {
    
    

    return -1;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
