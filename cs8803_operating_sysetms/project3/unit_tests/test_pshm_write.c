/*  

Program to test POSIX shared memory IPC. This segment writes to the
shared memory. The porpose of the program is to illustrate the basic
steps needed to write to  shared memory: get file descriptor -->
truncate the file to be "size" bytes --> perform VA to PA mapping -->
and use the allocated memory.

Compile: gcc -Wall test_pshm_write.c -o test_pshm_write -lrt
Run: ./test_pshm_write /shm_name "<string within quotes>"
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
    size_t len;
    void *addr;

    if (argc != 3) {
        printf("[main] Usage: %s <string beginning with forward slash> \"<string within quotes>\"\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /* Delete previous instance of shm object if it exists. This will
    prevent segfault when you try to create the memory with the same
    name when it already exists */

    shm_unlink(argv[1]);

    /* Get size from command line argument */
    len = strlen(argv[2]);

    /* Get file descriptor for the shared memory segment */
    fd = shm_open(argv[1], O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);

    /* Resize the memory segment to requested size */
    ftruncate(fd, len);

    /* Get the virtual address of the allocated memory */
    addr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    close(fd); 
    
    /* Put some data in the memory */
    memcpy(addr, argv[2], len);

    return 0;
}