/*
libcurl hello world

Compilation:
gcc -Wall -L/usr/lib/x86_64-linux-gnu test_libcurl.c -lcurl

Usage: 
./a.out "<url withing quotes>"

Notes:
1. The library path (the path following the -L option) is obtained by curl-config --libs
2. the "-lcurl" goes *after* the source file

Created by: Rohan Kekatpure
Date: 3/21/2015
*/


#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>

#define STATUS_FAIL -1
#define STATUS_SUCCESS 0

int main (int argc, char *argv[]) {

    /* Check number of arguments ls*/
    if (argc != 2) {
        printf("Usage: ./a.out <\"url_within_quotes\">\n");
        printf("Example: ./a.out \"http://www.google.com\"\n");
        exit(STATUS_FAIL);
    }    

    /* Global initialization */
    CURLcode glb_init_status = curl_global_init(CURL_GLOBAL_ALL);
    
    if (glb_init_status < 0) {
        printf("[main] Error: Global initialization failed\n");
        exit(STATUS_FAIL);
    }

    /* Get the curl handle */
    CURL *curl =  curl_easy_init();

    /* Error out if initialization failed */
    if (!curl) {
        printf("[main] Error: Curl initialization failed\n");
        exit(STATUS_FAIL);
    }

    /* create the URL */
    char *url = argv[1];

    /* Create the result object*/
    CURLcode res;

    /* Set the URL */
    curl_easy_setopt(curl, CURLOPT_URL, url);

    /* "get" the resource */
    res = curl_easy_perform(curl);

    /* cleanup*/
    curl_easy_cleanup(curl);

    return STATUS_SUCCESS;
}