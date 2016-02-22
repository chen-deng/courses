#ifndef _TEST_PSHM_XFR
#define _TEST_PSHM_XFR
#include <semaphore.h>
#include <stdio.h>

#define KBYTE 1024
#define BUF_SIZE 4 * KBYTE

typedef struct {
    char buf[BUF_SIZE];          /* 4 KiB buffer */
    sem_t sem_writer;           /* writer semaphore */
    sem_t sem_reader;           /* reader semaphore */
    int cnt;                     /* Indicator for file read complete */
} shmbuf;

void err_exit(char *context, char* msg, int exit_status) {
    printf("[%s] Error: %s\n", context, msg);
    exit(exit_status);
}

void warn(char *context, char* msg){
    printf("[%s] Warning: %s\n", context, msg);
}

#endif