/*

POSIX message queue consumer

Compilation: make -f make_pshm_xfr test_pshm_xfr_writer
Run: test_pshm_xfr_writer /<shm_name> <file_to_transfer>

Date: 3/24/2015
Author: Rohan Kekatpure

*/

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <mqueue.h>
#include <stdlib.h>
#include <unistd.h>

#define MQ_MSG_SIZE 8192

/* Utility functions */
void err_exit(char *context, char* msg, int exit_status) {
    printf("[%s] Error: %s\n", context, msg);
    exit(exit_status);
}

void strerr_exit(char *context, int errnum) {   
    printf("[%s] Error: %s\n", context, strerror(errnum));
    exit(errnum);
}

int main(int argc, char *argv[]) {

    /* Validate cmd line arguments */
    if (argc != 2) {
        printf("[usage] ./test_pmsg_send </queue_name>\n");
        err_exit("main", "Incorrect arguments", 1);
    }

    /* Open message queue created by sender */
    mqd_t mqd;
    mqd = mq_open(argv[1], O_RDONLY);
    if (mqd == (mqd_t) -1) 
        strerr_exit("main, mq_open", errno);

    /* get message length */
    struct mq_attr attr;
    if (mq_getattr(mqd, &attr) == -1)
        strerr_exit("main, mq_getattr", errno);

    /* Read sender message and echo to output, in loop */
    char msg[MQ_MSG_SIZE];
    while (1) {
        memset(msg, 0, 50);

        if (mq_receive(mqd, msg, MQ_MSG_SIZE, 0) == -1)
            strerr_exit("main, mq_recv", errno);
        printf("%s (%zu)\n", msg, attr.mq_msgsize);
    }
    
    return 0;
}