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

#include "gfserver.h"
#include "shm_channel.h"
#include "steque.h"
                                                                \
#define USAGE                                                                 \
"usage:\n"                                                                    \
"  webproxy [options]\n"                                                     \
"options:\n"                                                                  \
"  -p [listen_port]    Listen port (Default: 8888)\n"                         \
"  -t [thread_count]   Num worker threads (Default: 1, Range: 1-1000)\n"      \
"  -s [server]         The server to connect to (Default: Udacity S3 instance)"\
"  -z [shm_segsize]    Size in bytes of shared memory segment (Default: 4096)" \
"  -n [shm_segcount]   Number of shared memory segments (Default: 100)"        \
"  -h                  Show this help message\n"                              \
"special options:\n"                                                          \
"  -d [drop_factor]    Drop connects if f*t pending requests (Default: 5).\n"


/* Options object */
static struct option gLongOptions[] = {
  {"port",          required_argument,      NULL,           'p'},
  {"thread-count",  required_argument,      NULL,           't'},
  {"server",        required_argument,      NULL,           's'},
  {"shm_segsize",   required_argument,      NULL,           'z'},
  {"shm_segcount",  required_argument,      NULL,           'n'},         
  {"help",          no_argument,            NULL,           'h'},
  {NULL,            0,                      NULL,             0}
};

extern ssize_t handle_with_cache(gfcontext_t *ctx, char *path, void *shmd_p);

static gfserver_t gfs;

/* Global variables, shared between threads */
mqd_t mqd;                              /* message queue desc*/
char *mq_name = "/mq_proxy_cache";      /* message queue name */
steque_t shm_segnum_queue;              /* Shared memory seg id queue */
int shm_segcount = 2;                   /* shared memory segment count */
pthread_mutex_t sq_mtx;                 /* mutex to protect segid queue */
pthread_cond_t sq_notempty;             /* CV to signal that segnum queue is not empty */
int num_writers = 0;
int num_readers;

/* Helper functions */
static void _sig_handler(int signo);

/* main */
int main(int argc, char **argv) {
    int option_char = 0;
    unsigned short port = 8888;
    unsigned short nworkerthreads = 1;
    char *server = "s3.amazonaws.com/content.udacity-data.com";
    size_t shm_segsize = 4 * KBYTE;
    fprintf(stderr, "server = %s\n", server);

    if (signal(SIGINT, _sig_handler) == SIG_ERR){
        fprintf(stderr,"Can't catch SIGINT...exiting.\n");
        exit(EXIT_FAILURE);
    }

    if (signal(SIGTERM, _sig_handler) == SIG_ERR){
        fprintf(stderr,"Can't catch SIGTERM...exiting.\n");
        exit(EXIT_FAILURE);
    }

    /* Parse and set command line arguments */
    while ((option_char = getopt_long(argc, argv, "p:t:s:z:n:h", gLongOptions, NULL)) != -1) {
        switch (option_char) {
            case 'p': // listen-port
              port = atoi(optarg);
                break;
            case 't': // thread-count
                nworkerthreads = atoi(optarg);
                break;
            case 's': // file-path
                server = optarg;
                break;
            case 'z': // shm segment size
                shm_segsize = atoi(optarg);
                break;
            case 'n': // shm segment count 
                shm_segcount = atoi(optarg);
                break;                                                                          
            case 'h': // help
                fprintf(stdout, "%s", USAGE);
                exit(0);
                break;       
            default:
                fprintf(stderr, "%s", USAGE);
                exit(1);
        }
    }
  
    /* Initialize steque to hold shm segment ids */
    steque_init(&shm_segnum_queue);

    /* SHM initialization...*/
    /* Initially spawn 100 threads */    
    long shm_segnum;
    int shm_fd;
    char shm_segid[SEGID_LEN];

    /* total size of shared memory segment is size of the struct + requested segment size */    
    size_t shm_blocksize = sizeof(Shm_Block) + shm_segsize;
    printf("shm_blocksize = %zu\n", shm_blocksize);

    for (shm_segnum = 0; shm_segnum < shm_segcount; shm_segnum++) {
        sprintf(shm_segid, "/%ld", shm_segnum);

        /* Steps to create SHM segment id */  

        /* Delete previous shm instance of same name */
        if (shm_unlink(shm_segid) == -1) {
            //warn("main", "No previous shared memory instance found");
        }

        /* Create file descritor */
        shm_fd = shm_open(shm_segid, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
        if (shm_fd == -1) {
            err_exit("webproxy", "shm_open", errno);
        }

        /* adjust the memory region to size of shmbuf */
        if (ftruncate(shm_fd, shm_blocksize) == -1) {
            err_exit("main", "ftruncate", errno);
        }
        
        /* map the memory */
        Shm_Block *shmb_p;
        shmb_p = mmap(NULL, shm_blocksize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
        if (shmb_p == MAP_FAILED) {
            err_exit("main", "mmap failed", errno);
        }

        /* Push the segnum into the queue; because steque performs 
        only a shallow copy, we need to push elemental types, not 
        pointers. */
        steque_push(&shm_segnum_queue, (steque_item)shm_segnum);

        /* close the SHM file descriptors, they're no longer needed */   
        close(shm_fd);
    }

    /* Set up message queue attrs */
    struct mq_attr attr;
    attr.mq_maxmsg = MQ_MAXMSG;
    attr.mq_msgsize = MQ_MSGSIZE;

    /* Open message mqueue with specified attributes */
    mqd = mq_open(mq_name, O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR, &attr);
    if (mqd == (mqd_t) -1){
        strerr_exit("webproxy, mq_open", errno);
    }
    
    /* Initialize mutex and cv to protect segqueue*/
    pthread_mutex_init(&sq_mtx, NULL);
    pthread_cond_init(&sq_notempty, NULL);

    /* Initializing server */
    gfserver_init(&gfs, nworkerthreads);

    /* Setting options */
    gfserver_setopt(&gfs, GFS_PORT, port);
    gfserver_setopt(&gfs, GFS_MAXNPENDING, 10);
    gfserver_setopt(&gfs, GFS_WORKER_FUNC, handle_with_cache);

    /* Spawn threads */
    num_writers = 0;
    num_readers = nworkerthreads;

    long *th_ids = (long *) malloc(nworkerthreads * sizeof(long)); 
    int i;
    for(i = 0; i < nworkerthreads; i++) {
        th_ids[i] = i;
        // fprintf(stderr, "Assigning thread id %ld\n", th_ids[i]);

        /* Construct workload context to pass on to the handler */
        Workload wld;
        wld.shm_blocksize = shm_blocksize;
        wld.mqd = mqd;                      /* populate mq file desc */
        wld.segqueue = &shm_segnum_queue;   /* populate segment queue id */
        wld.sq_mtx_p = &sq_mtx;             /* populate the mutex */
        wld.sq_notempty_p = &sq_notempty; /* populate cv */

        gfserver_setopt(&gfs, GFS_WORKER_ARG, i, &wld);
    }

    free(th_ids);
    /* Loops forever */
    gfserver_serve(&gfs);

}

//=============================================

static void _sig_handler(int signo){
    int shm_segnum;
    char shm_segid[SEGID_LEN];
    if (signo == SIGINT || signo == SIGTERM){
        fprintf(stderr, "Received KILL signal\n");
        

        /* Free the queue */
        pthread_mutex_lock(&sq_mtx);
        steque_destroy(&shm_segnum_queue);
        pthread_mutex_unlock(&sq_mtx);

        /* Clean up mutex and condition variables */
        // pthread_mutex_destroy(&sq_mtx);
        // pthread_cond_destroy(&sq_notempty);

        /* Remove shared memory segments */
        for (shm_segnum = 0; shm_segnum < shm_segcount; shm_segnum++) {
            /* Unlink shared memory ids and mark them for deletion */
            sprintf(shm_segid, "/%d", shm_segnum);
            shm_unlink(shm_segid);
            fprintf(stderr, "[_sig_handler] Terminated shm_segid %s\n", shm_segid);
        }

        /* Close and unlink message queue */
        close(mqd);
        mq_unlink(mq_name);
        fprintf(stderr, "[_sig_handler] Terminated message queue %s\n", mq_name);

        gfserver_stop(&gfs);
        exit(signo);
  }
}
