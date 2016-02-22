#include <stdio.h>
#include "minifyjpeg.h"
#include "minifyjpeg_xdr.c"
#include "minifyjpeg_clnt.c"

CLIENT* get_minify_client(char *server) {
    CLIENT* clnt;
    if ((clnt = clnt_create(server, MINIFY_PROG, MINIFY_VERS, "tcp")) == NULL) {
        clnt_pcreateerror (server);
        fprintf(stderr, "clnt_create\n");
        exit(1);
    }
    return clnt;
}

void* minify_via_rpc(CLIENT* clnt, void* src_val, size_t src_len, size_t *dst_len){
    
    /* Populate src img structure */
    Img src;
    src.buf.buf_val = src_val;
    src.buf.buf_len = src_len;

    /* Allocate result image */
    Img *dstp = (Img *) malloc(sizeof(Img));
    dstp->buf.buf_val = (char *) malloc(src_len * sizeof(char));

    printf("before = %p\n", dstp->buf.buf_val);

    /* Call the remote procedure */
    ssize_t status;
    status = minifyjpeg_1(src, dstp, clnt);
    printf("after = %p\n", dstp->buf.buf_val);

    *dst_len = dstp->buf.buf_len;

    printf("status = %zu\n", status);
    printf("reduced_size = %zu\n", *dst_len);

    return dstp->buf.buf_val;       
}