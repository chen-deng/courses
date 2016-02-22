/*

POSIX message queue producer

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

#define KBYTE 1024 
#define MQ_MAXMSG 10

/* Utility functions*/
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

    /* Set message queue attributes */
    int msgsize = 4 * KBYTE;
    struct mq_attr attr;
    attr.mq_maxmsg = MQ_MAXMSG;
    attr.mq_msgsize = msgsize;

    /* Open message with specified attributes */
    mqd_t mqd;   
    mqd = mq_open(argv[1], O_CREAT | O_EXCL | O_WRONLY, S_IRUSR | S_IWUSR, &attr);
    if (mqd == (mqd_t) -1) {
        strerr_exit("main, mq_open", errno);
    }

    /* Read user input and send messages in infinite loop */
    char msg[50];
    while (1) {
        
        if (scanf("%s", msg) < 0)
            err_exit("main, scanf", "Error getting string", 1);

        if (mq_send(mqd, msg, strlen(msg), 0) == -1)
            strerr_exit("main, mq_send", errno);
    }
    
    return 0;
}

