#ifndef _SHM_SHANNEL_H
#define _SHM_SHANNEL_H

#include <getopt.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <mqueue.h>
#include <pthread.h>
#include "steque.h"
#include <semaphore.h>


#define KBYTE 1024
#define MAX_CACHE_REQUEST_LEN 256
#define SEGID_LEN 256
#define MQ_MSGSIZE 4 * KBYTE
#define MQ_MAXMSG 10
#define HDR_LEN 256

void err_exit(char *context, char* msg, int exit_status);
void strerr_exit(char *context, int errnum);
void warn(char *context, char* msg);

/* Workload structure passed to webproxy worker threads */
typedef struct {
    mqd_t mqd;                          /* Message queue desc */
    steque_t *segqueue;             /* pointer to segnum queue */
    pthread_mutex_t *sq_mtx_p;             /* pointer to segnum queue mutex */
    pthread_cond_t *sq_notempty_p;        /* pointer to cv */
    size_t shm_blocksize;
} Workload;

/* Shared memory buffer structure fo data transfer between cache and proxy */
typedef struct {
    sem_t sem_writer;           /* writer semaphore */
    sem_t sem_reader;           /* reader semaphore */    
    size_t file_size;            /* File size if it exists, -1 if not*/
    int cnt;                     /* Indicator for file read complete */
} Shm_Block;

#endif