#include <stdlib.h>
#include <fcntl.h>
#include <curl/curl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <mqueue.h>
#include <pthread.h>

#include "gfserver.h"
#include "shm_channel.h"
#include "steque.h"

ssize_t handle_with_cache(gfcontext_t *ctx, char *path, void *arg) {

    /* Get thread id */
    Workload *wld = (Workload *)arg;

    pthread_t tid = pthread_self();
    
    printf("entered %ld\n", tid);
    /* Sequence of steps to pop a segment number from the queue  */
    /* Read phase */ 
    /* Lock the mutex */
    pthread_mutex_lock(wld->sq_mtx_p);

    /* Wait until the queue is empty and writers are waiting */
    while(steque_isempty(wld->segqueue)) {
        fprintf(stderr, "[handle_with_cache, thread_id = %ld] Waiting for shm segment\n", tid);
        pthread_cond_wait(wld->sq_notempty_p, wld->sq_mtx_p);
    }

    /* Pop a segment number */
    long shm_segnum = (long) steque_pop(wld->segqueue);
    fprintf(stderr, "[handle_with_cache, thread_id = %ld] Popped shm segment %ld\n", tid, shm_segnum);

    /* unlock the mutex */
    fprintf(stderr, "[handle_with_cache, thread_id = %ld] Unlocking the mutex\n", tid); 
    pthread_mutex_unlock(wld->sq_mtx_p);
    
    /* convert the shm_segnum to shm_segid string */
    fprintf(stderr, "[handle_with_cache, thread_id = %ld] converting segnum to segid\n", tid);  
    char shm_segid[SEGID_LEN];
    memset(shm_segid, 0, SEGID_LEN);
    if (sprintf(shm_segid, "%ld", shm_segnum) < 0) {
        err_exit("handle_with_cache", "sprintf", 1);
    }   

    /* Open descriptor for shared memory region */    
    int shm_fd = shm_open(shm_segid, O_RDWR, 0);
    if (shm_fd == -1) {
        strerr_exit("handle_with_cache, shm_open", errno);
    }   
    
    /* obtain the pointer to shared memory */
    Shm_Block *shmb_p;
    shmb_p = mmap(NULL, wld->shm_blocksize, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shmb_p == MAP_FAILED) {
        err_exit("handle_with_cache", "mmap failed", errno);
    }   
    
    /* close shm file descriptor; its no longer needed */
    close(shm_fd);

    /* SHM data buffer starts after the shm metadata */
    char *shm_buf = (char *) shmb_p + sizeof(Shm_Block);

    /* Initialize writer (simplecached) semaphore to 1; initially available */
    if (sem_init(&shmb_p->sem_writer, 1, 1) == -1) {
        err_exit("handle_with_cache", "Unable to initialize writer semaphore.", errno);
    }        

    /* Initialize reader (handle_with_cache) semaphore to 0; initially unavailable */
    if (sem_init(&shmb_p->sem_reader, 1, 0) == -1) {
        err_exit("handle_with_cache", "Unable to initialize reader semaphore.", errno);
    }   
    
    /* Compose message to be sent to simplecached with URL, shm segment id and shm_blocksize */
    char msg[MQ_MSGSIZE];
    if(sprintf(msg, "%s %ld %zu", path, shm_segnum, wld->shm_blocksize) < 0 ){
        err_exit("handle_with_cache", "sprintf", 1);
    }

    /* Send message */  
    fprintf(stderr, "[handle_with_cache, thread_id = %ld] Sending message on mqueue\n", tid);   
    if (mq_send(wld->mqd, msg, strlen(msg), 0) == -1)
        strerr_exit("handle_with_cache, mq_send", errno);

    /* Lock the reader */
    if (sem_wait(&shmb_p->sem_reader) == -1){
        strerr_exit("handle_with_cache, sem_wait (hdr)", errno);
    }

    /* Read file size from simplecached */
    fprintf(stderr, "[handle_with_cache, thread_id = %ld] Reading file-size from simplecached\n", tid);
    int file_size = shmb_p->file_size;
    fprintf(stderr, "[handle_with_cache] Req path = %s, file_size = %d\n", path, file_size);
    /* send GFS header */

    if (file_size == -1) {
        fprintf(stderr, "[handle_with_cache, thread_id = %ld] FNF, about to lock mutex\n", tid);   
        pthread_mutex_lock(wld->sq_mtx_p);
        fprintf(stderr, "[handle_with_cache, thread_id = %ld] Enqueuing the segnum %ld\n", tid, shm_segnum);    
        steque_enqueue(wld->segqueue, (steque_item) shm_segnum);        
        pthread_mutex_unlock(wld->sq_mtx_p);        
        pthread_cond_broadcast(wld->sq_notempty_p);
        return gfs_sendheader(ctx, GF_FILE_NOT_FOUND, 0);
    }

    /* unlock the writer */
    if (sem_post(&shmb_p->sem_writer) == -1) {
        strerr_exit("handle_with_cache, sem_post (hdr)", errno);
    }

    gfs_sendheader(ctx, GF_OK, file_size);
    
    /* STEPS FOR FILE TRANSFER */
    /* We use a pair of semaphores to alternate control between reader (i.e. handle_with_cache)
    and the writer (i.e. simplecached). We also keep track of number of trnasfers and bytes 
    transferred */
    int nxfrs = 0;
    ssize_t bytes_xfrd = 0;

    for (nxfrs = 0, bytes_xfrd = 0 ;; nxfrs++, bytes_xfrd += shmb_p->cnt) {

        /* Lock the reader semaphore. If it is already locked, this call will block. Thus, the
        control will never go past this point unless the writer has unlocked the semaphore. */
        if (sem_wait(&shmb_p->sem_reader) == -1) {
            strerr_exit("handle_with_cache, sem_wait (data)", errno);
        }       

        /* File is done on the writer side */
        if (shmb_p->cnt == 0) {
            break;
        }

        /* Read Bytes from the shared memory */
        // printf("\n[handle_with_cache] writing data below \n\n");
        // write(STDOUT_FILENO, shm_buf, shmb_p->cnt);

        if (gfs_send(ctx, shm_buf, shmb_p->cnt) != shmb_p->cnt) {
            err_exit("handle_with_cache", "gfs_send", EXIT_FAILURE);
        }

        /* Unlock the writer */
        if (sem_post(&shmb_p->sem_writer) == -1) {
            strerr_exit("handle_with_cache, sem_post (data)", errno);
        }
    }

    /* Work is done; steps to put the shared memory segment back into queue */  
    fprintf(stderr, "[handle_with_cache, thread_id = %ld] FF, about to lock mutex\n", tid);   
    pthread_mutex_lock(wld->sq_mtx_p);
    fprintf(stderr, "[handle_with_cache, thread_id = %ld] Enqueuing the segnum %ld\n", tid, shm_segnum);    
    steque_enqueue(wld->segqueue, (steque_item) shm_segnum);
    pthread_mutex_unlock(wld->sq_mtx_p);
    pthread_cond_broadcast(wld->sq_notempty_p);

    return bytes_xfrd;
}

