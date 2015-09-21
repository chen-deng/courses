/*

This program reads contents of a file specified on command line and
places them in the shared memory for a reader to consume. The access
to the shared memory is alternated between he writer and the reader
using a pair of semaphores.

Compilation: make -f make_pshm_xfr test_pshm_xfr_writer
Run: test_pshm_xfr_writer /<shm_name> <file_to_transfer>

Date: 3/24/2015
Author: Rohan Kekatpure

*/

#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include "test_pshm_xfr.h"

int main(int argc, char *argv[]) {
    
    int source_fd;                 /* Desc for source file */

    /* Exit if incorrect number of args */
    if (argc != 3) {
        err_exit("main", "Usage: [fwd slath]<shm_name> <filename>", EXIT_FAILURE);
        exit(EXIT_FAILURE);
    }   
    
    /* Delete previous shm instance of same name */
    if (shm_unlink(argv[1]) == -1) {
        warn("main", "Unable to unlink");
    }

    /* Setup the shared memory segment */
    int map_fd = shm_open(argv[1], O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    if (map_fd == -1) {
        err_exit("main", "shm_open", errno);
    }
    
    /* adjust the memory region to size of shmbuf */
    // ftruncate(map_fd, sizeof(shmbuf));
    if (ftruncate(map_fd, sizeof(shmbuf)) == -1) {
        err_exit("main", "Unable to truncate shared memory file desc to requested size", errno);
    }
    
    shmbuf *shmp;                  /* Structure to hold file contents */
    shmp = mmap(NULL, sizeof(shmbuf), PROT_READ | PROT_WRITE, MAP_SHARED, map_fd, 0);

    if (shmp == MAP_FAILED) {
        err_exit("main", "mmap failed", errno);
    }
    
    /* Init writer semaphore; initially available */
    if (sem_init(&shmp->sem_writer, 1, 1) == -1) {
        err_exit("main", "Unable to initialize writer semaphore.", errno);
    }

    /* Init reader semaphore; initially unavailable */
    if (sem_init(&shmp->sem_reader, 1, 0) == -1) {
        err_exit("main", "Unable to initialize reader semaphore.", errno);
    }      

    /* Open file for reading */
    if ((source_fd = open(argv[2], O_RDONLY)) < 0) {
        err_exit("main","Unable to open source file descriptor.", errno);
    }

    int transfers = 0;
    int bytes_sent = 0;
    for (transfers = 0, bytes_sent = 0;;transfers++, bytes_sent += shmp->cnt) {    
        /* Lock writer semaphore */
        if (sem_wait(&shmp->sem_writer) == -1) {
            err_exit("main", "sem_wait failed in writer", errno);
        }

        /* We're reading BUF_SIZE bytes from the source file (which is an
        ordinary disk file)  and storing (writing) them to the shared memory
        buffer. So although the function call  below says "read" its actually
        "writing" to shared memory */ 
        shmp->cnt = read(source_fd, shmp->buf, BUF_SIZE);
        if (shmp->cnt == -1) {
            err_exit("main", "read failed", EXIT_FAILURE);
        }

        /* Unlock the reader */
        if (sem_post(&shmp->sem_reader) == -1) {
            err_exit("main", "Unable to unlock reader", errno);
        }

        if (shmp->cnt == 0) {
            break;
        }
    } 
    
    /* The reader is done and has given one more turn to clean up the IPC objects. */
    close(source_fd);
    close(map_fd);

    /* Destroy the writer semaphore */
    if (sem_destroy(&shmp->sem_writer) == -1) {
        err_exit("main", "sem_destroy failed for writer", errno);
    }
    
    /* Destroy the reader semaphore */
    if (sem_destroy(&shmp->sem_reader) == -1) {
        err_exit("main", "sem_destroy failed for reader", errno);
    }

    /* clean up the shared memory */
    if (shm_unlink(argv[1]) == -1) {
        err_exit("main", "Unable to unlink", errno);
    }

    printf("[main] Sent %d bytes (%d transfers)\n", bytes_sent, transfers);
    return 0;
}
