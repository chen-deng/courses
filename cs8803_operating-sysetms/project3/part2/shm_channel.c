//In case you want to implement the shared memory IPC as a library...
#include "shm_channel.h"

/* Exit with error */
void err_exit(char *context, char* msg, int exit_status) {
    fprintf(stderr, "[%s] Error: %s\n", context, msg);
    exit(exit_status);
}

/* print warning message */
void warn(char *context, char* msg){
    fprintf(stderr, "[%s] Warning: %s\n", context, msg);
}

/* print string error */
void strerr_exit(char *context, int errnum) {   
    fprintf(stderr, "[%s] Error: %s\n", context, strerror(errnum));
    exit(errnum);
}