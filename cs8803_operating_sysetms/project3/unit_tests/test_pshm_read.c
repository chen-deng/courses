/* 

Program to test POSIX shared memory IPC. This program reads from the
shared memory. The main purpose is to illustrate steps in reading from
an existing memory.

Compile: gcc -Wall test_pshm_read.c -lrt
Run: ./test_pshm_read /shm_name

date: 3/23/2015
author: Rohan Kekatpure

*/
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int fd;
    void *addr;
    struct stat sb;

    if (argc != 2) {
        printf("[main] Usage: %s <string beginning with forward slash>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Get file descriptor for the shared memody segment */
    fd = shm_open(argv[1], O_RDONLY, 0);

    /* Get the size of the memory */
    fstat(fd, &sb);

    /* Get the virtual address of the allocated memory */
    addr = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);

    /* 'fd' is no longer needed */
    close(fd);

    /* Put some data in the memory */
    write(STDOUT_FILENO, addr, sb.st_size);

    return 0;
}