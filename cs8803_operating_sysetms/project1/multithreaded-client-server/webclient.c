/* Implements a multithreaded client (load generator) using Boss-worker pattern */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h> 
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h> 
#include <pthread.h>
#include <libgen.h>
#include <netdb.h>
#include <netinet/in.h>

#define KBYTES 1024                 /* definition of a kilobyte*/
#define BUF_SIZE 32 * KBYTES        /* buffer size used for file read and socket communication */
#define REQ_SIZE 250                /* length of a client request = max path length (200) + "GetFile" + "FILE_NOT_FOUND */
#define PATH_LEN 200                /* max expected length of path to a file */
#define NUM_THREADS 1               /* default # threads; used if -t option is absent */
#define NUM_REQS 10                 /* default # requests; used if -r option is absent */
#define SERVER_HOST "127.0.0.1"     /* default host ip; used if -s option is absent */
#define SERVER_PORT 8888            /* default port; used if -p option is absent */
#define WKLD_FILE "./workload.txt"  /* default workload file; used if -w option is absent */
#define MTRC_FILE "./metrics.txt"   /* default metrics directory; used if -m option is absent */
#define DEBUG_LEVEL 1               /* Make debug level 0 if you dont want any messages on stdout */

/* round robin queue */
typedef struct {
    char **buf;     /* buffer to hold array of strings */
    int head;       /* pointer to track writes */
    int tail;       /* pointer to track reads */
    int size;       /* size of the queue */
} RRQueue;



/* The workload object. Has things needed by a thread to generate the workload */
typedef struct {
    int thread_id;
    long host;                  /* host or ip */
    int port;                   /* port */   
    int num_tasks;              /* number of requests to be serviced by this thread */
    int dflag;                  /* flag to indicate if a downloads directory is provided */
    char dwnld_dir[PATH_LEN];   /* Path to downloads directory */
} Workload;


/* Functions for RRQueue management and their helper functions */
RRQueue *rrqueue_init(size_t size); 
char *rrqueue_pop(RRQueue *q);
void rrqueue_push(RRQueue *q, char *elem);
void rrqueue_free(RRQueue *q);
int file_to_queue(char *file_name, RRQueue *qp);
size_t count_lines(char *file_name);

/* Helper functions for the worker thread */
int connect_to_server(Workload* w);                 /* Connects to server and returns the socket descriptor */
int send_request(int socket_desc, char *message);   /* Sends getfile request to server */
int recv_data(int socket_desc, Workload *wl, char* file_path); /* receives data */
void *worker (void *arg); /* runs the above functions */

/* utilities */
void print_help();
float time_diff(struct timeval *t_start, struct timeval *t_end);

/* mutexes and global variable for shared queue */
RRQueue *glb_req_files_queue;   /* pointer to the roundrobin resources queue */
pthread_mutex_t mtx_queue;      /* mutex for the shared queue */

/* mutex and global variable for received bytes and request servicing time */
long glb_tot_recv_bytes;
long glb_tot_req_time;
pthread_mutex_t mtx_metrics; 


/* main (Boss) */
int main(int argc, char* argv[]) {

    /* Parse command line arguments */
    int c; 

    int sflag = 0;
    char server_host[PATH_LEN] = SERVER_HOST;
    long server_host_ip;

    int pflag = 0;
    int server_port = SERVER_PORT;

    int tflag = 0;
    int num_threads = NUM_THREADS;

    int wflag = 0;
    char workload_file[PATH_LEN] = WKLD_FILE;

    int dflag = 0;
    char dwnld_dir[PATH_LEN] = "";

    int rflag = 0;
    int num_requests = NUM_REQS;

    int mflag = 0;
    char metrics_file[PATH_LEN] = MTRC_FILE;

    /* Variables for computing and storing metrics*/
    FILE *mf;                       /*metrics file */
    struct timeval t_start, t_end;  /* timval structs to hold before and after times */
    float elapsed_time;             /* elapsed time in sec */

    /* Variables to convert hostname into numeric ip address */
    struct hostent *he;

    opterr = 0;
    while ((c = getopt(argc, argv, ":s:p:t:w:d:r:m:h")) != -1) {
        switch(c) {
            case 's':
                sflag = 1;
                strcpy(server_host, optarg);
                break;
            case 'p':
                pflag = 1;
                server_port = atoi(optarg);                
                if (server_port <= 0) {
                    print_help();
                    printf("[main] Invalid server port (must be > 0, preferably > 1024)\n");
                    exit(EXIT_FAILURE);
                }
                break;            
            case 't':
                tflag = 1;
                num_threads = atoi(optarg);                
                if (num_threads <= 0) {
                    printf("[main] Invalid number of threads (must be > 0)\n");
                    exit(EXIT_FAILURE);
                }
                break;                                
            case 'w':
                wflag = 1;
                strcpy(workload_file, optarg);
                break;
            case 'd':
                dflag = 1;
                strcpy(dwnld_dir, optarg);
                break;
            case 'r':
                rflag = 1;
                num_requests = atoi(optarg);                
                if (num_requests <= 0) {
                    print_help();
                    printf("[main] Invalid number of requests (must be > 0)\n");
                    exit(EXIT_FAILURE);
                }
                break;                                
            case 'm':
                mflag = 1;
                strcpy(metrics_file, optarg);            
                break;
            case 'h':
                print_help();
                exit(EXIT_SUCCESS);
                break;
            case '?':
                printf("[main] unknown option '%c'\n", optopt);
                exit(EXIT_FAILURE);
                break;
            case ':':
                printf("[main] option '%c' requires an argument\n", optopt);
                print_help();
                exit(EXIT_FAILURE);
                break;
            default:
                break;

        }
    }

    if (!sflag || !pflag || !tflag || !wflag || !rflag || !mflag ) {
        printf ("[main] Some options not provided...defaults used\n");
    }

    printf("[main] server:                  %s\n", server_host);
    printf("[main] port:                    %d\n", server_port);
    printf("[main] num_threads:             %d\n", num_threads);
    printf("[main] num_requests:            %d\n", num_requests);
    printf("[main] downloads directory:     %s\n", dwnld_dir);
    printf("[main] workload file:           %s\n", workload_file);
    printf("[main] metrics file:            %s\n", metrics_file);

    /* Get hostent structure for the server address */
    he = gethostbyname(server_host);

    /* Exit if couldnt get hostent structure for the specified server */
    if (he == NULL) {
        printf("[main] could not get ip address for host %s\n", server_host);
        return -1;
    }

    /* Extract the ip address */
    server_host_ip = *(unsigned long *)(he->h_addr_list[0]);

    /* If num_threads > num_requests, make num_threads = num_requests */
    if (num_threads > num_requests) {
        printf("[main] num_threads > num_requests. Making num_threads = %d\n", num_requests);
        num_threads = num_requests;
    }

    Workload *wl = (Workload *) malloc(num_threads * sizeof(Workload)); /* workload objects to be passed to threads */
    pthread_t *th_workers = (pthread_t *) malloc(num_threads * sizeof(pthread_t)); /* Worker thread pool*/
    int i; 

    /* Create the round robin queue containing filenames */
    glb_req_files_queue = rrqueue_init(count_lines(workload_file));
    file_to_queue(workload_file, glb_req_files_queue);
    
    /* Initialize the mutex */
    pthread_mutex_init(&mtx_queue, NULL);
    pthread_mutex_init(&mtx_metrics, NULL);

    /* Initialize metrics */
    glb_tot_recv_bytes = 0;

    /* Spawn threads */
    for (i = 0; i < num_threads; i++) {

        /* Create a copy of the workload object for each thread. */
        wl[i].thread_id = i;
        wl[i].host = server_host_ip;
        wl[i].port = server_port;        
        wl[i].num_tasks = num_requests/num_threads;
        wl[i].dflag = dflag;
        strcpy(wl[i].dwnld_dir, dwnld_dir);

        pthread_create(&th_workers[i], NULL, worker, (void *) &wl[i]);        
    }

    /* Threads are created, start the timer for elapsed time. Elapsed time is calculated in the 
    Boss thread.
    */
    gettimeofday(&t_start, NULL);

    /* Wait for threads to finish their work */
    for (i = 0; i < num_threads; ++i) {
        pthread_join(th_workers[i], NULL);
    }    

    /* Stop the timer for elapsed time */
    gettimeofday(&t_end, NULL);

    /* Calculate the elapsed time */
    elapsed_time = time_diff(&t_start, &t_end);


    /* Write the metrics to the metrics file */
    mf = fopen(metrics_file, "w");
    if (mf == NULL) {
        printf("[main] Error opening metrics file.\n");
        return EXIT_FAILURE;
    }

    /* Collect and print metrics. This is a critical section because of access to 
    glb_tot_recv_bytes and glb_tot_req_time variables. All the threads should have joined by now,
    so there shouldnt be really any race condition, but we want to be conservative. */
    pthread_mutex_lock(&mtx_metrics);
    fprintf(mf, "Webclient stats report:\n");
    fprintf(mf, "\tElapsed time        = %0.6f seconds\n", elapsed_time);
    fprintf(mf, "\tBytes received      = %ld bytes\n", glb_tot_recv_bytes);
    fprintf(mf, "\tThroughput          = %0.1f bytes/sec\n", (float) glb_tot_recv_bytes/elapsed_time);
    fprintf(mf, "\tRequests serviced   = %d\n", num_requests);
    fprintf(mf, "\tRequest throughput  = %0.1f req/sec\n", (float)num_requests/(float)elapsed_time);    
    fprintf(mf, "\tAvg response time   = %0.6f sec\n", (float)glb_tot_req_time/(float)num_requests);
    pthread_mutex_unlock(&mtx_metrics);

    /* clean up */
    pthread_mutex_destroy(&mtx_queue);
    pthread_exit(NULL); 
    rrqueue_free(glb_req_files_queue);   
    fclose(mf);
    free(wl);
    free(th_workers);
    return EXIT_SUCCESS;
}

/* worker thread function */
void *worker (void *arg) {
    Workload *wl = (Workload *) arg;        /* workload object for this thread */
    int socket_desc;                        /* socket descriptor */
    char request[REQ_SIZE];                 /* max expected length of a request string */
    char *req_file_path = NULL;             /* full path to the requested file; it will be obtained from the round-robin queue */
    int num_handled = 0;                    /* counter for number of requests handled so far; final value should equal R/T */
    int recv_bytes;                         /* bytes received in a single request */
    long thr_recv_bytes = 0;                /* total bytes received by this thread in num_handled requests */ 
    long thr_req_time = 0;                  /* total time spent by this thread in servicing num_handled requests */
    struct timeval t_req_start, t_req_end;  /* timeval structs for recodgin start and end times */

    while (num_handled < wl->num_tasks) {

        /* connect to server, return on error; no point in proceeding forward */
        socket_desc = connect_to_server(wl);
        if (socket_desc == -1) {
            printf("[worker][thread_id %d] Cannot connect to server\n", wl->thread_id);
            return NULL;
        }

        /* Initialize request and path strings */
        strcpy(request, "GetFile GET ");
        req_file_path = NULL;

        /* pop a filename from queue */
        pthread_mutex_lock(&mtx_queue);        
        req_file_path = rrqueue_pop(glb_req_files_queue);
        pthread_mutex_unlock(&mtx_queue);

        /* create request message, it is null terminated by virtue of strcat. The null termination 
        is required as per protocol spec */
        strcat(request, req_file_path);

        /* Send request to the server */
        send_request(socket_desc, request);

        /* Start timer */
        gettimeofday(&t_req_start, NULL);

        /* Receive data, including the header */
        recv_bytes = recv_data(socket_desc, wl, req_file_path);
    
        /* Stop timer */
        gettimeofday(&t_req_end, NULL);

        /* Update the total bytes received */
        thr_recv_bytes += recv_bytes;

        /* Update the total time spent in servicing requests */
        thr_req_time += time_diff(&t_req_start, &t_req_end);

        /* Update the number of requests handled */
        ++num_handled;

        /* clean up */    
        free(req_file_path);
        close(socket_desc);

    }

    /* Once all requests are serviced, update global total received bytes and total request time */
    pthread_mutex_lock(&mtx_metrics);
    glb_tot_recv_bytes += thr_recv_bytes;
    glb_tot_req_time += thr_req_time;        
    pthread_mutex_unlock(&mtx_metrics);

    pthread_exit(NULL);
    return NULL;
}

/* 
Receives header and data from the server. 
On success: returns the number of bytes received from the server. 
On Failure: returns -1 
*/
int recv_data(int socket_desc, Workload *wl, char* file_path) {
    
    /* If "-d" argument is provided, extract the basename from the requested file path,
    and construct the new path with basename appended to the downloads folder. */
    char *file_name;
    char new_file_path[PATH_LEN];
    FILE *fp = NULL;
    
    char protocol[100];
    char status[100];

    int i;

    /* count of spaces in the header. 3 spaces indicate end of header and beginning of file data */
    int space_count = 0; 

    /* memory location in the header where the file data begins */
    int data_index; 
    
    /* Number of bytes in the header that belong to file data; these need to be copied to the 
    download file if -d option is specified */
    int copy_bytes; 

    /* Status variable to indicate if the requested file was found by the server. It is set to 1 if 
    the received server response header doesnt say FILE_NOT_FOUND */
    int file_found_status = 0; 

    size_t bytes_written;   /* bytes written to output file */    
    size_t file_size;       /* file_size in server response */    
    char *head_buf;         /* header buffer */    
    char *data_buf;         /* data buffer is allocated dynamically */    
    size_t hdr_bytes = 0;   /* Total bytes in header */    
    int hdr_recv = 0;       /* bytes received so far*/    
    size_t left_in_hdr;     /* bytes remaining in head_buf after parsing out the header */    
    size_t data_bytes = 0;  /* total bytes in data */    
    int data_recv;          /* bytes received so far */
    
    /* allocate and create head_buf */
    head_buf = (char *) malloc(BUF_SIZE * sizeof(char));
    if (head_buf == NULL) {
        printf("[recv_data] Could not allocate memory for header\n");
        exit(1);
    }
    // memset(head_buf, 0, BUF_SIZE); 

    /* loop and receive complete header */
    hdr_bytes = 0;
    while (hdr_bytes < BUF_SIZE) {
        hdr_recv = recv(socket_desc, head_buf + hdr_bytes, BUF_SIZE - hdr_bytes, 0);

        if (hdr_recv == 0) {
            break;
        }

        if (hdr_recv < 0 && errno == EAGAIN) {
            printf("[recv_data][thread_id %d] Error %d: %s\n", 
                wl->thread_id, errno, strerror(errno));
            continue;
        }

        if (hdr_recv < 0) {
            printf("[recv_data][thread_id %d] Error %d: %s\n", 
                wl->thread_id, errno, strerror(errno));
        }

        hdr_bytes += hdr_recv;
    }

    /* parse out protocol, status and filesize */
    sscanf(head_buf, "%s %s %zu", protocol, status, &file_size);
    
    /* Set the file_found_status to 1 or 0 based on whether file was found by the server */
    if (strcmp(status, "FILE_NOT_FOUND") == 0) {
        file_found_status = 0;
    } else if (strcmp(status, "OK") == 0) {
        file_found_status = 1;
    }

    /* If the file was not found, nothing to do. So notify and return with success. */
    if ( (file_found_status == 0) && (file_size == 0)) {
        free(head_buf);
        printf("[recv_data][thread_id %d] %s ... FILE_NOT_FOUND\n", wl->thread_id, file_path);
        return 0; 
    }

    if ( (int)file_size < 0) {
        free(head_buf);
        printf("[recv_data][thread_id %d] negative file_size received in header\n", wl->thread_id);
        return -1;
    }

    /* If "-d" argument is provided, and the file is found, extract the basename from the 
    requested file path, and construct the new path with basename appended to the downloads folder. 
    Take care to insert a "/" between the downloads folder and basename. Multiple "/" are treated 
    as one and dont cause problems. 
    */
    if ((1 == wl->dflag) && (1 == file_found_status)) {
        file_name = basename(file_path);
        strcpy(new_file_path, wl->dwnld_dir);
        strcat(new_file_path, "/");
        strcat(new_file_path, file_name);
        fp = fopen(new_file_path, "wb");
        if (fp == NULL) {
            printf("[recv_data][thread_id %d] Error opening download file: %s\n", 
                wl->thread_id, new_file_path);
        }        
    }

    /* File found and filesize > 0, so allocate memory for data_buf */    
    data_buf = (char *) malloc(BUF_SIZE * sizeof(char));

    /* Error out of the function if unable to create data buffer. This should be fairly rare
    since we're requesting only 32 Kbytes */
    if (data_buf == NULL) {
        printf("[recv_data] error allocating memory\n");
        free(head_buf);
        return -1;
    } 

    /* Parse out the header and copy remaining data to data_buf. We detect three spaces and mark it
    as end of theh header. The rest of the bytes in the header are actual file data. If the -d 
    switch is given, we got to write this data to the output file. If not, we can throw it away. */
    for (i = 0; i < BUF_SIZE; ++i) {
        if (head_buf[i] == ' ') {
            ++space_count;
        }

        if (space_count == 3) {
            data_index = i + 1;
            break;
        }
    } 

    /* If you didnt detect three spaces till the end of the header buffer, the header is incorrect.
    So return and error; no point proceeding forward */
    if ( (i >= BUF_SIZE - 1) || (data_index == 0)) {
        printf("[recv data] Incorrect header format: %s\n", head_buf);
        free(head_buf);
        free(data_buf);
        return -1;
    }

    /* calculate bytes left in header after the header is extracted. You really have to convince 
    yourself of this equation by drawing 32 boxes! */
    left_in_hdr = BUF_SIZE - data_index;

    /* If the entire data was contained in the header, just copy "file_size" bytes from current 
    location. Otherwise, you need to copy left_in_hdr bytes from end_of_header location  */
    if (file_size < left_in_hdr) {
        copy_bytes = file_size;
    } else {
        copy_bytes = left_in_hdr;
    }
 
    /* If -d was specified and file was opened, write data bytes in head_buf to the output file */
    if ((wl->dflag == 1) && (fp != NULL)) {
        fflush(fp);
        bytes_written = fwrite(head_buf + data_index, sizeof(char), copy_bytes, fp);
    }

    /* We've already received "copy_bytes" bytes; so we start the counter there. We need to loop
    until we receive "file_size" bytes, i.e., until data_bytes == file_size */
    data_bytes = copy_bytes;
    while (data_bytes < file_size) {
        data_recv = recv(socket_desc, data_buf, BUF_SIZE, 0);
        if (data_recv == 0) {
            break;
        }

        data_bytes += data_recv;

        if ((wl->dflag == 1) && (fp != NULL)) {
            bytes_written += fwrite(data_buf, sizeof(char), data_recv, fp);
        }
    }

    /* After the loop, if you didnt receive "file_size" bytes, there must be some error.
    But dont exit yet. */
    if (data_bytes != file_size) {
        printf("[recv_data][thread_id %d] %s ... Warning: received = %zu, expected = %zu, lost = %d\n", 
            wl->thread_id, file_path, data_bytes, file_size, (int)(file_size - data_bytes));

    } else if (DEBUG_LEVEL) {
        printf("[recv_data][thread_id %d] %s ... Success\n", wl->thread_id, file_path);
    }   

    if (wl->dflag == 1) {
        if (bytes_written != data_bytes) {
            printf("[recv_data][thread_id %d] Warning: bytes_received = %ld, bytes_written = %ld\n", 
                wl->thread_id, data_bytes, bytes_written);
        }    
    }

    free(data_buf);
    free(head_buf);
    if (fp != NULL) { 
        fclose(fp); 
    };

    return data_bytes;
}

/* sends request to server.
On success: returns 0
On failure: returns -1 
*/
int send_request(int socket_desc, char *message) {
    int bytes_sent;
    /* strlen(message) would NOT count the null byte at the end. But you need to send that per 
    the protocol spec */
    bytes_sent = send(socket_desc, message, strlen(message) + 1, 0);

    /* Error oout if number of bytes < 0 */
    if (bytes_sent < 0) {
        fprintf(stderr, "[send_response_header] %s\n", strerror(errno));
        return -1;
    }    

    return 0;    
}

/* Creates socket and connects to server. Returns socket descriptor. */
int connect_to_server(Workload* w) {
    int socket_desc;
    struct sockaddr_in sock; 

    /* Error out if couldnt create socket */
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1) {
        return -1;
    }

    /* populate server parameters */
    sock.sin_addr.s_addr = w->host;
    sock.sin_family = AF_INET;
    sock.sin_port = htons(w->port);

    /* connect to server; error out if couldnt connect */
    if (connect(socket_desc, (struct sockaddr*) &sock, sizeof(sock)) < 0) {
        return -1;
    } 
    return socket_desc;
    
}

/* Utility function: count number of non-empty lines in a fil. This function is required to load
the workload file in the round-robin queue */
size_t count_lines(char *file_name) {

    int num_lines = 0;
    char *line = NULL;
    size_t len = 0;
    size_t nchars;
    FILE *fp = fopen(file_name, "r");

    if (fp == NULL) {
        fprintf(stderr, "[count_lines] cannot open file\n");
        exit(EXIT_FAILURE);
    }

    while ((nchars = getline(&line, &len, fp)) != -1) {
        /* ignore empty lines */
        if (nchars == 1)
            continue;

        ++num_lines;
    }   
    fclose(fp);
    return num_lines;
}


/* Read lines from file and push it to a round-robin queue */
int file_to_queue(char *file_name, RRQueue *qp) {
    char *line = NULL;
    size_t len = 0;
    size_t nchars;

    FILE *fp = fopen(file_name, "r");
    if (fp == NULL) 
        return EXIT_FAILURE;

    while ((nchars = getline(&line, &len, fp)) != -1) {
        /* ignore empty lines */
        if (nchars == 1)
            continue;

        /* strip newline at end */
        if (line[nchars - 1] == '\n') {
            line[nchars - 1] = '\0';
            --nchars;
        }
        rrqueue_push(qp, line);
    }   
    fclose(fp);
    return EXIT_SUCCESS;
}

/* initialization of RRQueue */
RRQueue *rrqueue_init(size_t size) {
    size_t i; 

    RRQueue *q = (RRQueue *) malloc(sizeof(RRQueue));
    q->head = 0;
    q->tail = 0;
    q->size = size; 

    /* allocate the buffer */
    q->buf = (char **) malloc(size * sizeof(char *));

    /* allocate space for each string in the buffer */
    for (i = 0; i < q->size; i++){
        q->buf[i] = (char *) malloc(PATH_LEN * sizeof(char));
    }
    return q;
}

/* popping an element */
char *rrqueue_pop(RRQueue *q) {
    
    size_t len = strlen(q->buf[q->tail]);
    char *retval = (char *) malloc((len + 1) * sizeof(char));
    strcpy(retval, q->buf[q->tail]);
    q->tail++;

    /* Wrap around if at the end */
    if (q->tail == q->size) {
        q->tail = 0;
    }

    return retval;
}

/* push an element */
void rrqueue_push(RRQueue *q, char *elem) {
    /* ignore elem if the queue is full */
    if (q->head == q->size) {
        return; 
    }

    strcpy(q->buf[q->head], elem);
    q->head++;
    return;
}

/* free the queue */
void rrqueue_free(RRQueue *q) {
    int i;

    for (i = 0; i < q->size; i++){
        free(q->buf[i]);
    }
    free(q->buf);
    free(q);
}

void print_help() {
    printf("usage:\n ./webclient [options]\n");
    printf("options:\n");
    printf("\t-s [server_addr]    Server address (Default: 0.0.0.0)\n");
    printf("\t-p [server_port]    Server port (Default: 8888)\n");
    printf("\t-t [thread_count]   Num worker threads (Default: 1, Range: 1-100)\n");
    printf("\t-w [workload_path]  Path to workload file (Default: workload.txt)\n");
    printf("\t-d [download_dir]   Path to downloaded file directory (Default: null)\n");
    printf("\t-r [request_count]  Num total requests (Default: 10, Range: 1-1000)\n");
    printf("\t-m [metrics_path]   Path to metrics file (Default: metrics.txt)\n");
    printf("\t-h                  Show this help message\n");                    
}

float time_diff(struct timeval *t_start, struct timeval *t_end) {
    float sec_elapsed;
    float USEC = 1000000.0;
    sec_elapsed = (t_end->tv_sec - t_start->tv_sec)
                + (float)(t_end->tv_usec - t_start->tv_usec)/ USEC;
    return sec_elapsed;
}