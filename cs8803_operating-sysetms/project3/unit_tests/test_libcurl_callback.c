/*
* libcurl with callback
* 
* Compilation:
* gcc -Wall -L/usr/lib/x86_64-linux-gnu test_libcurl_callback.c -lcurl
* 
* Usage: 
* ./a.out "<url withing quotes>"
* 
* Notes:
* 1. The library path (the path following the -L option) is obtained by curl-config --libs
* 2. the "-lcurl" goes *after* the source file
* 
* Created by: Rohan Kekatpure
* Date: 3/21/2015
*/ 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define STATUS_FAIL -1
#define STATUS_SUCCESS 0
#define KBYTE 1024

/* Memory chunk structure */
typedef struct {
    char *memory;
    size_t size;    
} Memory_Chunk;

/* Call back function to aggregate the data 
* Note that curl_easy_perform() is a *blocking* get function. So when the callback is invoked 
* we already have information about the data size. We merely need to allocate the required
* size 
*/

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    /* Calculate actual data size */
    size_t realsize = size * nmemb;

    /* Cast the user specified structure into Memory_Chunk */
    Memory_Chunk *mem = (Memory_Chunk *) userp;        

    /* Allocate the memory based on calculated actual size */
    mem->memory = realloc(mem->memory, mem->size + realsize + 1); // why mem->size + realsize + 1 ?
    
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

int main (int argc, char *argv[]) {
    
    char *url = malloc(200 * sizeof(char));

    /* Check number of arguments ls*/
    if (argc != 2) {
        printf("[main] Usage: ./a.out <\"url_within_quotes\">\n");
        printf("[main] Example: ./a.out \"http://www.google.com\"\n");
        strcpy(url, "http://s3.amazonaws.com/content.udacity-data.com/courses/ud923/filecorpus/road.jpg");
        printf("[main] Info: Using default url: %s\n", url);
    } else {
        strcpy(url, argv[1]);        
    }

    CURLcode res;                       /* Create the result object */
    CURL *curlh = NULL;                 /* curl handle */
    Memory_Chunk chunk;                 /* memory chunk */
    char errbuf[4 * KBYTE];             /* Buffer to store memory */

    /* Initialize chunk */
    chunk.memory = malloc(1);
    chunk.size = 0;
    
    /* Global initialization of curl */
    CURLcode glb_init_status = curl_global_init(CURL_GLOBAL_ALL);
    
    if (glb_init_status < 0) {
        printf("[main] Error: Global initialization failed\n");
        exit(STATUS_FAIL);
    }

    /* Error out if initialization failed */
    curlh = curl_easy_init();
    if (!curlh) {
        printf("[main] Error: Curl initialization failed\n");
        exit(STATUS_FAIL);
    }

    /* Set the curl options */
    curl_easy_setopt(curlh, CURLOPT_URL, url);
    curl_easy_setopt(curlh, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curlh, CURLOPT_WRITEDATA, (void*) &chunk);
    curl_easy_setopt(curlh, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    curl_easy_setopt(curlh, CURLOPT_ERRORBUFFER, errbuf);

    /* "get" the resource */
    res = curl_easy_perform(curlh);

    /* Print the result status */
    printf("status = %u\n", res);

    /* check for errors */
    if (res != CURLE_OK) {
        printf("[main] Error: curl_easy_perform failed (error code %u)\n", res);
        printf("[main] Error message: %s\n", errbuf);
        exit(STATUS_FAIL);
    }

    /* print the file information */
    printf("[main] Size of the downloaded file is: %zu bytes\n", chunk.size);
    
    /* save result to file */
    FILE *fout;
    fout = fopen("saved_image.jpg", "wb");
    fwrite(chunk.memory, sizeof(char), chunk.size, fout);

    /* cleanup */
    free(url);
    free(chunk.memory);
    curl_easy_cleanup(curlh);

    return STATUS_SUCCESS;
}

