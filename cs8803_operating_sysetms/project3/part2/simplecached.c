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
#include <sys/types.h>
#include <pthread.h>

#include "shm_channel.h"
#include "simplecache.h"

/* Global_variables */
mqd_t mqd;
char *mq_name = "/mq_proxy_cache";
pthread_t *th_workers; /* pointer to the array of worker threads */

/* Signal handler */
static void _sig_handler(int signo){
    if (signo == SIGINT || signo == SIGTERM){
        /* gracefully exit pthreads */
        fprintf(stderr, "[_sig_handler] Destroying threads.\n");
        free(th_workers);  /* free is not async signal safe */

        /* Close message queue file descriptor */
        fprintf(stderr, "[_sig_handler] Closing message queue file descriptor.\n");
        close(mqd);
        exit(signo);
    }
}

#define USAGE                                                                 \
"usage:\n"                                                                    \
"  simplecached [options]\n"                                                  \
"options:\n"                                                                  \
"  -t [thread_count]   Num worker threads (Default: 1, Range: 1-1000)\n"      \
"  -c [cachedir]       Path to static files (Default: ./)\n"                  \
"  -h                  Show this help message\n"                              

/* Options object */
static struct option gLongOptions[] = {
    {"nthreads",           required_argument,      NULL,           't'},
    {"cachedir",           required_argument,      NULL,           'c'},
    {"help",               no_argument,            NULL,           'h'},
    {NULL,                 0,                      NULL,             0}
};

void Usage() {
    fprintf(stdout, "%s", USAGE);
}

/* Thread worker function */
void *worker_func(void* arg);

/* main (Boss thread) */
int main(int argc, char **argv) {
    int nthreads = 1;
    char *cachedir = "locals.txt";
    char option_char;

    while ((option_char = getopt_long(argc, argv, "t:c:h", gLongOptions, NULL)) != -1) {
        switch (option_char) {
            case 't': // thread-count
                nthreads = atoi(optarg);
                break;   
            case 'c': //cache directory
                cachedir = optarg;
                break;
            case 'h': // help
                Usage();
                exit(0);
                break;    
            default:
                Usage();
                exit(1);
        }
    }

    if (signal(SIGINT, _sig_handler) == SIG_ERR){
        fprintf(stderr,"Can't catch SIGINT...exiting.\n");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGTERM, _sig_handler) == SIG_ERR){
        fprintf(stderr,"Can't catch SIGTERM...exiting.\n");
        exit(EXIT_FAILURE);
    }

    /* Initializing the cache */
    simplecache_init(cachedir);

    /* Read messages from webproxy */   
    /* Open the message queue */
    while((mqd = mq_open(mq_name, O_RDONLY)) == -1) {
        /* Spin until message queue is open */
    }

    /* Allocate worker threads */
    th_workers = (pthread_t *) malloc (nthreads * sizeof(pthread_t));

    /* Array to hold thread ids; necessary to prevent race condition when assigning ids */
    long *th_ids = (long *) malloc(nthreads * sizeof(long)); 

    /* Spawn the threads */
    int idx = 0;
    long th_retval = 0;
    for (idx = 0; idx < nthreads; ++idx) {
        th_ids[idx] = idx;
        th_retval = pthread_create(&th_workers[idx], NULL, worker_func, (void*) &th_ids[idx]);
        if (th_retval > 0) {
            strerr_exit("simplecached, pthread_create", th_retval);
        }
    }

    /* Wait for the threads to join */
    for (idx = 0; idx < nthreads; idx++) {
        th_retval = pthread_join(th_workers[idx], NULL);
    }

    /* threads Ids are assigned; we no longer need array of Ids */
    free(th_ids);

    /* Exit pthreads */
    pthread_exit(NULL);
    free(th_workers);

    return 0;
}

void *worker_func(void* arg) {

    /* Parse message */
    char shm_segid[SEGID_LEN];
    char msg[MQ_MSGSIZE];
    long th_id = *(long*) arg;

    while(1) {
        /* Initialize space to receive message */
        memset(msg, 0, MQ_MSGSIZE);
    
        /* Receive message */
        if (mq_receive(mqd, msg, MQ_MSGSIZE, 0) == -1) {
            strerr_exit("worker_func, mq_receive", errno);
        }
        
        /* parse file URL, shm_segnum and shm_blocksize from the message */
        size_t shm_blocksize;
        char file_path[MAX_CACHE_REQUEST_LEN];
        long shm_segnum;        
        sscanf(msg, "%s %ld %zu", file_path, &shm_segnum, &shm_blocksize);
        
        /* Get the file descriptor for requested file */
        int req_fd = simplecache_get(file_path);

        /* Get the file size */
        struct stat req_finfo;
        int file_size;
        if (req_fd > 0) {
            if (fstat(req_fd, &req_finfo) == -1) {
                strerr_exit("worker_func, fstat", errno);
            }
            file_size = req_finfo.st_size;
        } else {
            /* file not found in cache */
            file_size = -1;                            
        }

        /* convert shm_segnum to shm_segid */
        memset(shm_segid, 0, SEGID_LEN);
        if (sprintf(shm_segid, "%ld", shm_segnum) < 0) {
            err_exit("worker_func", "sprintf", 1);
        }

        /* Open descriptor for shared memory region */
        int shm_fd = shm_open(shm_segid, O_RDWR, 0);
        if (shm_fd == -1) {
            strerr_exit("worker_func, shm_open", errno);
        }   
        
        /* obtain the pointer to shared memory */
        Shm_Block *shmb_p;
        shmb_p = mmap(NULL, shm_blocksize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (shmb_p == MAP_FAILED) {
            err_exit("worker_func", "mmap failed", errno);
        }   
        
        /* close shm file descriptor; its no longer needed */
        close(shm_fd);

        /* SHM data buffer starts after the shm metadata */
        char *shm_buf = (char *) shmb_p + sizeof(Shm_Block);

        /* Lock writer semaphore */
        if (sem_wait(&shmb_p->sem_writer) == -1) {
            strerr_exit("worker_func, sem_wait (hdr)", errno);
        }
        
        /* Update file_size in shared memory */
        shmb_p->file_size = file_size;
        fprintf(stderr, "[worker_func, thread %ld, shm_segnum = %ld] Req path = %s, file_size = %d\n", 
            th_id, shm_segnum, file_path, file_size);
        
        /* Unlock the reader */
        if (sem_post(&shmb_p->sem_reader) == -1) {
            strerr_exit("worker_func, sem_post (hdr)", errno);
        } 

        /* If file was not found in the cache, ignore the following data transfer steps */
        if (file_size == -1) {
            continue;
        }

        /* STEPS FOR FILE TRANSFER */     
        int nxfrs = 0;
        ssize_t bytes_xfrd = 0;
        off_t offset = 0;

        for (nxfrs = 0, bytes_xfrd = 0 ; ; nxfrs++, bytes_xfrd += shmb_p->cnt, offset += shmb_p->cnt) {            

            /* Lock the writer semaphore. After the header is read, and the file is found, the 
            reader (i.e. handle_with_cache) will unlock the writer (i.e. simplecached). Otherwise
            the execution will be blocked at the following "sem_wait" */
            if (sem_wait(&shmb_p->sem_writer) == -1) {
                strerr_exit("simplecached, sem_wait (data)", errno);
            }

            /* Copy the data from the file into the buffer */
            shmb_p->cnt = pread(req_fd, shm_buf, shm_blocksize, offset); 
            // write(STDOUT_FILENO, shm_buf, shmb_p->cnt);

            if (shmb_p->cnt == -1) {
                strerr_exit("simplecached, read", errno);
            }

            /* Unlock the reader (handle_with_cache) */
            if (sem_post(&shmb_p->sem_reader) == -1) {
                strerr_exit("simplecached, sem_post (data)", errno);
            }

            /* If no more bytes to read, break */
            if (shmb_p->cnt == 0) {
                break;
            }
        }
    }
    
    return NULL;
}












