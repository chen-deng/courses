/*

This program reads contents of the shared memory location specified on
the command line. The access to the shared memory is alternated
between he writer and the reader using a pair of semaphores.

Compilation: make -f make_pshm_xfr test_pshm_xfr_reader
Run: test_pshm_xfr_reader /<shm_name>

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

    /* Exit if wrong number of arguments */
    if (argc != 2) {
        err_exit("main", "Usage: ./a.out [forward slash]<shm_name>", EXIT_FAILURE); 
    }   

    /* Open *existing* shared memory location specified on command line */
    int map_fd = shm_open(argv[1], O_RDWR, 0); /* mode_t is 0 if we're accessing an existing shm object */
    if (map_fd == -1) {
        err_exit("main", "shm_open failed", errno);
    }
    
    /* Attach memory allocated by writer to virtual address space of this process */
    shmbuf *shmp = mmap(NULL, sizeof(shmbuf), PROT_READ | PROT_WRITE, MAP_SHARED, map_fd, 0);
    if (shmp == MAP_FAILED) {
        err_exit("main", "mmap failed", errno);
    }

    /* Print the size of the allocated virtual memory region */
    /*
    struct stat sb;
    fstat(map_fd, &sb);
    printf("[main] Size of the shared memory segment: %lu \n", sb.st_size);
    printf("[main] Size of shmbuf = %ld\n", sizeof(shmbuf));
    printf("[main] Size of components: buf = %ld, sem_reader = %ld, sem_writer = %ld, int = %ld\n",sizeof(shmp->buf), sizeof(shmp->sem_reader), sizeof(shmp->sem_writer), sizeof(shmp->cnt));
    */
   
    int transfers = 0;
    int bytes_sent = 0;
    for (transfers = 0, bytes_sent = 0;;transfers++, bytes_sent += shmp->cnt) { 

        /* Lock the reader */
        if (sem_wait(&shmp->sem_reader) == -1) {
            err_exit("main", "sem_wait failed in reader", errno);
        }

        /* File is done on the writer side */
        if (shmp->cnt == 0) {
            break;
        }
        
        /* Read cnt bytes from shared memory */
        /* Even though the function call above says "write" we're actually "read"-ing from 
        shared memory buffer and writing to STDOUT*/
        if (write(STDOUT_FILENO, shmp->buf, shmp->cnt) != shmp->cnt) {
            err_exit("main", "partial write", errno);
        }

        /* Unlock the writer */
        if (sem_post(&shmp->sem_writer) == -1) {
            err_exit("main", "sem_post failed", errno);
        }
    }

    /* Give one more turn to writer so it can clean up */
    if (sem_post(&shmp->sem_writer) == -1) { 
        err_exit("main", "sem_post failed", errno);
    }

    /* clean up */
    close(map_fd);

    printf("[main] Received %d bytes (%d transfers)\n", bytes_sent, transfers);
    return 0;
}