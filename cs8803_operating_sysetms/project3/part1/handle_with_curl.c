/*

handle_with_curl is a callback function that converts incoming GetFile
(a fictitious protocol) requests into HTTP requests using libcurl C
API. The concept is pretty simple. The webproxy connects  to a
specified server on the interwebs (Amazon S3 by default). The client
specifies the relative path of the resource using the Getfile
protocol. Finally the proxy server fetches the resource by appending
the relative path to the server to which it is connected to.

handle_with_curl uses write_callback as a helper function.
write_callback  writes the contents fetched by CURL into a data
structure of users' choice.

Date: 3/22/2015
Author: Rohan D. Kekatpure

 */
#include <stdlib.h>
#include <fcntl.h>
#include <curl/curl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "gfserver.h"

#define STATUS_FAIL -1
#define STATUS_SUCCESS 0
#define KBYTE 1024

#define STATUS_FAIL -1
#define STATUS_SUCCESS 0

/* Memory block structure */
typedef struct {
    char *memory;
    size_t size;    
} Block;

/*
 
Call back function for `curl_easy_perform` to aggregate the data. This
function receives the source data in "contents". "size" is the size of
each item in bytes and "nmemb" is the number of items, giving a total
size in bytes of nmemb * size. Pointer userp is a custom data
structure in which the raw data in "contents" can be pushed.

Note that curl_easy_perform() is a *blocking* get function. So when
the callback is invoked we already have information about the data
size. We merely need to allocate the required size

 */

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    /* Calculate actual data size */
    size_t realsize = size * nmemb;

    /* Cast the user specified structure into Block */
    Block *mem = (Block *) userp;        

    /* Allocate the memory based on calculated actual size */

    mem->memory = realloc(mem->memory, mem->size + realsize + 1);
    
    /* Error out if realloc fails */
    if (mem->memory == NULL) {
        printf("[write_callback] Error: realloc returned NULL\n");
        return STATUS_FAIL;
    }

    /* Copy "realsize" bytes from "contents" to "mem->memory" */
    memcpy(&(mem->memory[mem->size]), contents, realsize);

    /* update size */
    mem->size += realsize;

    /* Null terminate the memory */
    mem->memory[mem->size] = 0;

    return realsize;
}

/*
 Function for handling getfile requests with curl
*/
ssize_t handle_with_curl(gfcontext_t *ctx, char *file_path, void* server_addr){

    CURLcode res;                       /* Create the result object */
    CURL *curlh = NULL;					/* curl handle */
    Block block;                 /* memory block */
    char resource_url[4 * KBYTE];       /* string to hold full resource path (server addr + file path) */
    char errbuf[4 * KBYTE];             /* Buffer to store memory */


    /* Memory block to hold file contents. It will be realloc'ed to data size in the callback */
    block.memory = malloc(1);
    block.size = 0;

    /* Error out if initialization failed */
    curlh = curl_easy_init();
    if (!curlh) {
        printf("[handle_with_curl] Error: Curl initialization failed\n");
        exit(STATUS_FAIL);
    }

    /* Construct full path to resource */
    strcpy(resource_url, (char *) server_addr);
    strcat(resource_url, file_path);
    printf("[handle_with_curl] resource_path: %s\n", resource_url);

    /* Set the curl options */
    curl_easy_setopt(curlh, CURLOPT_URL, resource_url);
    curl_easy_setopt(curlh, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curlh, CURLOPT_WRITEDATA, (void*) &block);
    curl_easy_setopt(curlh, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curlh, CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curlh, CURLOPT_FAILONERROR, -1);

    /* "get" the resource */
    res = curl_easy_perform(curlh);

    /* check for errors */
    if (res != CURLE_OK) {
        printf("[handle_with_curl] Error: curl_easy_perform failed (error code %u)\n", res);
        printf("[handle_with_curl] Error message: %s\n", errbuf);
        free(block.memory);
        curl_easy_cleanup(curlh);
        return gfs_sendheader(ctx, GF_FILE_NOT_FOUND, 0);
    }

    /* print the file information */
    printf("[handle_with_curl] Size of the downloaded file is: %zu bytes\n", block.size);

	/* Send header */   
	gfs_sendheader(ctx, GF_OK, block.size);

	/* Send data */
	int bytes_transferred = 0;
	int write_len = 0;

	while(bytes_transferred < block.size){
		write_len = gfs_send(ctx, block.memory, block.size);
		if (write_len != block.size){
			fprintf(stderr, "[handle_with_curl] Error: write error (error code %d)\n", errno);
			return EXIT_FAILURE;
		}
		bytes_transferred += write_len;
	}

    /* cleanup */
    free(block.memory);
    curl_easy_cleanup(curlh);

	return bytes_transferred;
}

