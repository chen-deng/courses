#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "minifyjpeg.h"
#include "magickminify.h"

#define DEBUG 0

/* Implement the needed server-side functions here */
bool_t minifyjpeg_1_svc(Img src, Img *result,  struct svc_req *rqstp)
{
    /* Initialize the magickminify library */
    magickminify_init();

    /* Minify the image */
    ssize_t res_len;
    char *res_val = (char*) magickminify(src.buf.buf_val , src.buf.buf_len, &res_len);
    fprintf(stderr, "size after magickminify = %d\n", res_len);
    
    /* Point result to the pointer returned by magickminify */
    result->buf.buf_val = res_val;
    result->buf.buf_len = res_len;
    
    fprintf(stderr, "orig size = %d, reduced size = %d\n", src.buf.buf_len, result->buf.buf_len);

    if (DEBUG) {
        /* Write image to a local file */

        /* Open the file */
        int fp;
        if ((fp = open("./server_img.jpg", O_WRONLY|O_CREAT, S_IRUSR | S_IWUSR)) == -1) {
            fprintf(stderr, "open\n");
            fprintf(stderr, "%s\n", strerror(errno));
            exit(1);
        }

        /* Write data */
        if (write(fp, res_val, res_len) != res_len) {
            fprintf(stderr, "write\n");
            exit(1);
        }

        close(fp);
    }

    magickminify_cleanup();
    fprintf(stderr, "returning from minifyjpeg.c\n");
    return 1;
}

int minify_prog_1_freeresult (SVCXPRT *p, xdrproc_t xdrproc, caddr_t res) {
    xdr_free(xdrproc, res);
    return 1;
}