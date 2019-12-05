/* disk.c: SimpleFS disk emulator */

#include "sfs/disk.h"
#include "sfs/logging.h"

#include <fcntl.h>
#include <unistd.h>

/* Internal Prototyes */

bool    disk_sanity_check(Disk *disk, size_t blocknum, const char *data);

/* External Functions */

/**
 *
 * Opens disk at specified path with the specified number of blocks by doing
 * the following:
 *
 *  1. Allocates Disk structure and sets appropriate attributes.
 *
 *  2. Opens file descriptor to specified path.
 *
 *  3. Truncates file to desired file size (blocks * BLOCK_SIZE).
 *
 * @param       path        Path to disk image to create.
 * @param       blocks      Number of blocks to allocate for disk image.
 *
 * @return      Pointer to newly allocated and configured Disk structure (NULL
 *              on failure).
 **/
Disk *	disk_open(const char *path, size_t blocks) {

    int new_fd = open(path, O_RDWR | O_CREAT, S_IRWXU);
    if(new_fd == -1) {
        fprintf(stderr, "Error opening file: %s\n", strerror(errno));
        return NULL;
    }

    if(ftruncate(new_fd, blocks*BLOCK_SIZE) == -1) {
        fprintf(stderr, "Error truncating file: %s\n", strerror(errno));
        return NULL;
    }

    Disk *d = calloc((size_t)1, sizeof(Disk));
    d->blocks = blocks;
    d->fd = new_fd;

    return d;
}

/**
 * Close disk structure by doing the following:
 *
 *  1. Close disk file descriptor.
 *
 *  2. Report number of disk reads and writes.
 *
 *  3. Releasing disk structure memory.
 *
 * @param       disk        Pointer to Disk structure.
 */
void	disk_close(Disk *disk) {

    if(close(disk->fd) == -1) {
        fprintf(stderr, "Error closing file: %s\n", strerror(errno));
    }
    
    printf("DISK READS:  %lu\nDISK WRITES: %lu\n", disk->reads, disk->writes);

    free(disk);

}

/**
 * Read data from disk at specified block into data buffer by doing the
 * following:
 *
 *  1. Performing sanity check.
 *
 *  2. Seeking to specified block.
 *
 *  3. Reading from block to data buffer (must be BLOCK_SIZE).
 *
 * @param       disk        Pointer to Disk structure.
 * @param       block       Block number to perform operation on.
 * @param       data        Data buffer.
 *
 * @return      Number of bytes read.
 *              (BLOCK_SIZE on success, DISK_FAILURE on failure).
 **/
ssize_t disk_read(Disk *disk, size_t block, char *data) {

    if(!disk_sanity_check(disk, block, data)) {
        return DISK_FAILURE;
    }

    int new_fd = disk->fd;
    if(lseek(new_fd, block*BLOCK_SIZE, SEEK_SET) == -1) {
        fprintf(stderr, "Error seeking block: %s\n", strerror(errno));
        return DISK_FAILURE;
    }

    if(read(new_fd, data, BLOCK_SIZE) == -1) { // args here may be wrong
        fprintf(stderr, "Error writing to file: %s\n", strerror(errno));
        return DISK_FAILURE;
    }

    disk->reads++;

    return BLOCK_SIZE;
}

/**
 * Write data to disk at specified block from data buffer by doing the
 * following:
 *
 *  1. Performing sanity check.
 *
 *  2. Seeking to specified block.
 *
 *  3. Writing data buffer (must be BLOCK_SIZE) to disk block.
 *
 * @param       disk        Pointer to Disk structure.
 * @param       block       Block number to perform operation on.
 * @param       data        Data buffer.
 *
 * @return      Number of bytes written.
 *              (BLOCK_SIZE on success, DISK_FAILURE on failure).
 **/
ssize_t disk_write(Disk *disk, size_t block, char *data) {

    if(!disk_sanity_check(disk, block, data)) {
        return DISK_FAILURE;
    }

    int new_fd = disk->fd;
    if(lseek(new_fd, block*BLOCK_SIZE, SEEK_SET) == -1) {
        fprintf(stderr, "Error seeking block: %s\n", strerror(errno));
        return DISK_FAILURE;
    }

    if(write(new_fd, data, BLOCK_SIZE) == -1) { // args here may be wrong
        fprintf(stderr, "Error writing to file: %s\n", strerror(errno));
        return DISK_FAILURE;
    }

    disk->writes++;

    return BLOCK_SIZE;
}

/* Internal Functions */

/**
 * Perform sanity check before read or write operation:
 *
 *  1. Check for valid disk.
 *
 *  2. Check for valid block.
 *
 *  3. Check for valid data.
 *
 * @param       disk        Pointer to Disk structure.
 * @param       block       Block number to perform operation on.
 * @param       data        Data buffer.
 *
 * @return      Whether or not it is safe to perform a read/write operation
 *              (true for safe, false for unsafe).
 **/
bool    disk_sanity_check(Disk *disk, size_t block, const char *data) {

    if(!disk) return false;

    if(block >= disk->blocks) return false;

    if(!data) return false;

    return true;
}

/* vim: set expandtab sts=4 sw=4 ts=8 ft=c: */
