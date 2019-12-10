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
    if(block.super.magic_number == MAGIC_NUMBER) {
        printf("    magic number is valid\n");
    } else {
        printf("    magic number is invalid\n");

    }
    printf("    %u blocks\n"         , block.super.blocks);
    printf("    %u inode blocks\n"   , block.super.inode_blocks);
    printf("    %u inodes\n"         , block.super.inodes);

    /* Read Inodes */

    for(int j=1; j <= block.super.inode_blocks; j++) {
        
        if (disk_read(disk, j, block.data) == DISK_FAILURE) {
            return;
        }

        for(uint32_t i = 0; i < INODES_PER_BLOCK; i++) {
        
            if(block.inodes[i].valid == 1) {
                printf("Inode %u:\n", i);
                printf("    size: %u bytes\n", block.inodes[i].size);
                printf("    direct blocks:"); 
                for(uint32_t q = 0; q < POINTERS_PER_INODE; q++) {
                    if(block.inodes[i].direct[q]) printf(" %d", block.inodes[i].direct[q]);
                }
                printf("\n");
 
                if(block.inodes[i].indirect) {

                    Block ind;

                    printf("    indirect block: %d\n", block.inodes[i].indirect);

                    if (disk_read(disk, block.inodes[i].indirect, ind.data) == DISK_FAILURE) {
                        return;
                    }  

                    printf("    indirect data blocks:");

                    for(int q = 0; q < POINTERS_PER_BLOCK; q++) {
                        if(ind.pointers[q] != 0) printf(" %d", ind.pointers[q]);
                    }

                    printf("\n");
                }
            }

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

    if(fs->disk != NULL) { 
        return false;
    }

    Block s;

    s.super.magic_number = MAGIC_NUMBER;
    s.super.blocks = disk->blocks;
    
    if(disk->blocks % 10 == 0) {
        s.super.inode_blocks = disk->blocks / 10;
    } else {
        s.super.inode_blocks = (disk->blocks / 10) + 1;
    }

    s.super.inodes = s.super.inode_blocks * INODES_PER_BLOCK;

    if(disk_write(disk, 0, s.data) == DISK_FAILURE) return false;

    Block ib;
    for(uint32_t i = 1; i < disk->blocks; i++) {
        if(disk_write(disk, i, ib.data) == DISK_FAILURE) return false;
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

    if(s.super.blocks != disk->blocks) {
        return false;
    }

    if((s.super.inode_blocks != (disk->blocks / 10 )) && (s.super.inode_blocks != (disk->blocks / 10 ) + 1)) {
        return false;
    }

    if(fs->disk) {      // no double mount
        return false;
    }

    fs->disk=disk;
    fs->meta_data=s.super;

    bool *bitmap = malloc(disk->blocks * sizeof(bool));
    memset(bitmap, 1, disk->blocks * sizeof(bool));


    for(int i=0; i < s.super.inode_blocks + 1; i++) bitmap[i]=0; 

    Block table;
    Block ind;

    for(int q=1; q <= s.super.inode_blocks; q++) {

        if (disk_read(disk, q, table.data) == DISK_FAILURE) {
            return false;
        } 

        for(uint32_t i = 0; i < INODES_PER_BLOCK; i++) {

            if(table.inodes[i].valid == 1) { 

                // Check Direct pointers

                for(int q = 0; q < POINTERS_PER_INODE; q++) {
                    bitmap[table.inodes[i].direct[q]]=0;
                }

                // Check Indirect pointers

                if(table.inodes[i].indirect) {

                    bitmap[table.inodes[i].indirect]=0;

                    if (disk_read(disk, table.inodes[i].indirect, ind.data) == DISK_FAILURE) {
                        return false;
                    } 

                    for(int q = 0; q < POINTERS_PER_BLOCK; q++) {
                        bitmap[ind.pointers[q]]=0;
                    }
                }

            }

        }

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
    fs->free_blocks=NULL;

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

    Inode *i = NULL;
    if(!fs_load_inode(fs, inode_number, i)) {
        return false;
    }




    return true;
}

/**
 * Return size of specified Inode.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to remove.
 * @return      Size of specified Inode (-1 if does not exist).
 **/
ssize_t fs_stat(FileSystem *fs, size_t inode_number) {

    Inode *i = NULL;
    if(!fs_load_inode(fs, inode_number, i)) {
        return -1;
    }

    return i->size;
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
    
    Inode *i = NULL;
    if(!fs_load_inode(fs, inode_number, i)) {
        return -1;
    }
    
   // i->direct

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
    
    Inode *i = NULL;
    if(!fs_load_inode(fs, inode_number, i)) {
        return -1;
    }

    //disk_write(

    return -1;
}

/* Helper Functions */

/**
 * Load inode i with inode associated with number.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to load in i
 * @param       i               Inode struct.
 * @return      true if successful (false on error)
 **/
bool fs_load_inode(FileSystem *fs, size_t inode_number, Inode *node) {

    int block_num = (inode_number / INODES_PER_BLOCK) + 1; // which block to read from
    int i_num = inode_number % INODES_PER_BLOCK;

    Block s;

    if (disk_read(fs->disk, block_num, s.data) == DISK_FAILURE) {
            return false;
    }

    node=&(s.inodes[i_num]);

    return true;
}

/**
 * Save inode i with inode associated with number.
 *
 * @param       fs              Pointer to FileSystem structure.
 * @param       inode_number    Inode to save in i
 * @param       i               Inode struct.
 * @return      true if successful (false on error)
 **/

bool fs_save_inode(FileSystem *fs, size_t inode_number, Inode *node) {

    return true;
}


/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
