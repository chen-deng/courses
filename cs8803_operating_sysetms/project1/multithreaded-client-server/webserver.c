/* Single threaded file transfer via socket */
#include <stdio.h>              /* Standard IO */
#include <fcntl.h>              /* for open() */
#include <sys/socket.h>         /* Socket functions */
#include <sys/stat.h>           /* File statistics */
#include <sys/sendfile.h>       /* Sending file over a socket */
#include <arpa/inet.h>       
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>

/* Maximum message size expected from client */
#define KBYTES 1024             /* define a kilobyte */
#define BUF_SIZE 32 * KBYTES    /* buffer size used for file read and socket communication */
#define REQ_SIZE 250            /* length of a client request = max path length (200) + "GetFile" + "FILE_NOT_FOUND */      
#define MAX_PATH_LEN 200        /* max expected length of path to a file */
#define MAX_QUEUE_SIZE 1000     /* max size of the FIFO queue that holds client socket descriptors */
#define LISTEN_BACKLOG 250      /* socket listen backlog queue size; this is passed to the listen() function */
#define NUM_THREADS 1           /* default # server threads; used if -t option is absent */
#define SERVER_PORT 8888        /* default server port; used if -p option is absent */
#define DEBUG_LEVEL 1           /* Make debug level 0 if you dont want any messages on stdout */

/* Define a structure for holding client request object; has all information to service a 
client request */
typedef struct {
    int thread_id;
    char workload_dir[MAX_PATH_LEN];    /* file path */
    char file_name[MAX_PATH_LEN];       /* string to hold request file_name */
    off_t file_size;                    /* size of requested file */
} Asset;

/* FIFO queue to hold client connections; access to this queue is protected by a mutex */
typedef struct {
    int *buf;                   /* buffer to hold the queue data */
    int head;                   /* points to where next element can be pushed */
    int tail;                   /* points to where next element can be popped */
    int is_empty;               /* flag to indicate empty */
    int is_full;                /* flag to indicate full */
} Queue;

/* Functions to manipulate a queue; these modify the queue, so call only within a mutex-protected 
critical section */
Queue *queue_init(size_t size);
void queue_push(Queue *q, int in);
void queue_pop(Queue *q, int *out);
void queue_free(Queue *q);

/* Functions for communication with the client; receiving request and sending header + data */
int check_request (char *req);
int parse_request(char *request, Asset *rp);
int recv_request(int socket_desc, Asset *rp);
int send_response_header(int sock_desc, Asset *rp);
int send_file(int socket_desc, Asset *rp);

/* utilities */
void print_help();

/* Thread function */
void *worker(void *arg);

/* Multi threading */
pthread_mutex_t qmtx;               /* Mutex for queue */
pthread_cond_t not_empty;           /* CV to signal that queue is not empty */
pthread_cond_t not_full;            /* CV to signal that queue is not full */

/* Initialize a FIFO queue of ints to hold socket descriptors from client connections */
Queue *sock_queue;

/* main (Boss thread) */
int main(int argc, char *argv[]){

    /* Parse command line options */

	int s_opt;                         /* server port */

	int pflag = 0;                     /* port */
	int server_port = SERVER_PORT;

	int tflag = 0;                     /*  # threads */
	int num_threads = NUM_THREADS; 

	int fflag = 0;
	char workload_dir[MAX_PATH_LEN] = ".";

    opterr = 0;
	while((s_opt = getopt(argc, argv, ":p:t:f:h")) != -1) {
        switch(s_opt) {
            case 'p':
                pflag = 1;
                server_port = atoi(optarg);                
                if (server_port <= 0) {
                    print_help();
                    printf("[main] Invalid server port\n");
                    exit(EXIT_FAILURE);
                }
                break;            
            case 't':
                tflag = 1;
                num_threads = atoi(optarg);                
                if (num_threads <= 0) {
                    printf("[main] Invalid number of threads\n");
                    exit(EXIT_FAILURE);
                }
                break;                                
            case 'f':
                fflag = 1;
                strcpy(workload_dir, optarg);
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

    /* Alert about usage of any default options */
    if (!pflag || !tflag || !fflag) {
        printf ("[main] Some options not provided...defaults used\n");
    }

    printf("[main] port:                    %d\n", server_port);
    printf("[main] num_threads:             %d\n", num_threads);
    printf("[main] workload dir:            %s\n", workload_dir);


    /* variables to hold socket creation logic */
    struct sockaddr_in server, client;    
    int socket_desc, client_socket, c;       
    int bind_status, errno;
    int tnum;
    
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    
    if (socket_desc == -1) {
        fprintf(stderr, "[main] Socket creation failed\n");
    } 
    
    /* prepare sockaddr_in structure */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(server_port);
   
    /* Bind the socket to an IP and port */
    bind_status = bind(socket_desc, (struct sockaddr *)&server, sizeof(server));
    if ( bind_status < 0) {
        fprintf(stderr, "[main] Bind failed: %s\n", strerror(errno));
        return EXIT_FAILURE;
    } 
    
    /* Listen on configured port */
    listen(socket_desc, LISTEN_BACKLOG); 
    
    /* Initialize mutex and condition variables */
    pthread_mutex_init(&qmtx, NULL);
    pthread_cond_init(&not_empty, NULL);
    pthread_cond_init(&not_full, NULL);

    /* Allocate threads */
    pthread_t *th_handler = (pthread_t *) malloc(num_threads * sizeof(pthread_t));

    /* Alocate thread assets. All info that threads need is can be found in their asset */
    Asset *th_asset = (Asset *) malloc(num_threads * sizeof(Asset));

    /* Initialize the socket queue. The queue needs to be initialized *before* spawning the
    threads. Otherwise you'll get segfault. */
    sock_queue = queue_init(MAX_QUEUE_SIZE);

    for(tnum = 0; tnum < num_threads; tnum++) {
        /* Populate the Asset for each thread. */
        th_asset[tnum].thread_id = tnum;
        strcpy(th_asset[tnum].workload_dir, workload_dir);
        strcpy(th_asset[tnum].file_name, "");
        th_asset[tnum].file_size = 0;

        /* Create (spawn) the thread */
        pthread_create(&th_handler[tnum], NULL, worker, (void *) &th_asset[tnum]);
    }

    printf("[main] started server with %d worker threads\n", num_threads);

    /* Listen on the port for any incoming connections */
    c = sizeof(struct sockaddr_in);
    while (1) {

        client_socket = accept(socket_desc, (struct sockaddr*) &client, (socklen_t*) &c);

        if (client_socket < 0) {
            fprintf(stderr, "[main] Accept failed");
            return EXIT_FAILURE;
        }
   
        /* Put the client socket in queue. This is a critical section because of access to the 
        shared queue. If the queue is full, the Boss has got to wait until it receives the signal 
        */
        pthread_mutex_lock(&qmtx);
        while (sock_queue->is_full) {
            pthread_cond_wait(&not_full, &qmtx);
        }
        
        /* Once the signal is received, push into the queue */
        queue_push(sock_queue, client_socket);        
        pthread_mutex_unlock(&qmtx);
        pthread_cond_signal(&not_empty);
        
    }

    /* Wait for the threads to join */
    for (tnum = 0; tnum < num_threads; ++tnum) {
        pthread_join(th_handler[tnum], NULL);
    }    

    /* clean up and exit */
    pthread_mutex_destroy(&qmtx);
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);
    pthread_exit(NULL); 

    printf("[main] closing server socket. \n");
    close(socket_desc);
    queue_free(sock_queue);
    free(th_handler);
    free(th_asset);
    return EXIT_SUCCESS;
}

/* Entry point for worker threads  */
void *worker(void *arg){

    Asset *asset = (Asset *) arg;
    int client_socket; 

    while (1) {

        /* initialize requested filename. It will be available only after parsing the client 
        request */
        strcpy(asset->file_name, ""); 
        asset->file_size = 0;

        /* Critical section: Have to wait for queue-not-empty signal */
        pthread_mutex_lock(&qmtx);
        while (sock_queue->is_empty) {
            pthread_cond_wait(&not_empty, &qmtx);
        }

        /* Once wait is finished, pop the top element */
        queue_pop(sock_queue, &client_socket);
        pthread_mutex_unlock(&qmtx);
        pthread_cond_signal(&not_full);

        /* Service the request */
        int recv_status = -1;                /* Status variable for receiving client request */
        int send_header_status = -1;         /* Status variable for response header */
        int send_file_status = -1;           /* Status variable for sending file */

        /* Receive client request, parse it and populate the relevant fields of the thread Asset */
        recv_status = recv_request(client_socket, asset);

        /* If debug info is requested, alert about what the thread is currently working on*/
        if(DEBUG_LEVEL) {
            printf("[worker][thread_id %d] Requested file %s\n", asset->thread_id, asset->file_name);
        }

        /* if request was not received properly, alert the user, close client_socket and ignore the 
        rest of the operations */
        if (recv_status != 0) {
            fprintf(stderr, "[worker][thread_id %d] Error receiving client request.\n", asset->thread_id);
            close(client_socket);
            continue;
        }

        /* If request was received properly, send response header */
        if (recv_status == 0) {
            send_header_status = send_response_header(client_socket, asset);
        }

        /* If response header wasnt sent properly, close client socket ignore the remaining block */
        if (send_header_status == -1) {
            fprintf(stderr, "[worker][thread_id %d] Error sending response header\n", 
                asset->thread_id);
            close(client_socket);
            continue;
        }

        /* File not found. Since we've sent the response header, we really dont have to print this
        to server logs. But keeping this snippet here for debug purposes if needed  */
        /*        
        if (send_header_status == -2) {
            fprintf(stderr, "[worker][thread_id %d] File not found\n", asset->thread_id);
        }
        */
        
        /* If the file was found (ie send_header_status == 0), send requested file/data */
        if (send_header_status == 0) {
            send_file_status = send_file(client_socket, asset);
            if (send_file_status != EXIT_SUCCESS) {
                fprintf(stderr, "[worker][thread_id %d] Error sending file.\n", 
                    asset->thread_id);
            }
        }

        /* Clean up */
        close(client_socket);
    }    

    pthread_exit(NULL);
    return NULL;
}


/* Receive client request; parses client request and populates the relevant fields of the thread 
Asset structure */
int recv_request(int socket_desc, Asset *ast) {

    int parse_status = -1; /* Status of parse_request() function */
    char *req_buf = (char *) malloc(REQ_SIZE * sizeof(char));
    size_t recv_bytes_tot, recv_bytes;
    int recv_status = -1;

    /* Receive client request. */
    recv_bytes_tot = 0;
    while (recv_bytes_tot < REQ_SIZE) {
        recv_bytes = recv(socket_desc, req_buf + recv_bytes_tot, REQ_SIZE - recv_bytes_tot, 0);

        if (check_request(req_buf) == 0) {
            recv_status = 0;
            break;
        }

        /* No more data to receive, but no error either. This condition checks for non-null 
        terminated requests from  the client, a condition that'd violate the GetFile protocol */
        if (recv_bytes == 0) {
            recv_status = -1;
            break;
        }

        if (recv_bytes < 0) {
            recv_status = -1;
            printf("[recv_request][thread_id %d] Error %d: %s\n", 
                ast->thread_id, errno, strerror(errno));
            break;
        }

        recv_bytes_tot += recv_bytes;
        printf("[recv_request][thread_id %d] req_bytes_tot = %zu\n", ast->thread_id, recv_bytes_tot);
    }

    /* Parse client request only if req was received successfully (recv_req_status == 0)
    and populate the relevant fields of the thread Asset structure */
    if (recv_status == 0) {
        parse_status = parse_request(req_buf, ast);
    }

    /* Echo client request */
    if (parse_status != EXIT_SUCCESS) {
        printf("[recv_request][thread_id %d] Error parsing request\n", ast->thread_id);

        /* Request received successfully, but not parsed successfully. 
        Revert the recv_status to -1 */
        recv_status = -1;
    }

    free(req_buf);
    return recv_status;
}

/* parses a request string */
int parse_request(char *request, Asset *ast){
    int num_vars;
    char protocol[REQ_SIZE] = "GetFile";
    char action[REQ_SIZE] = "GET";
    char file_name[MAX_PATH_LEN] = "";

    num_vars = sscanf(request, "%s %s %s", protocol, action, file_name);

    if (num_vars != 3) {
        printf("[parse_request] Could not parse protocol, action and file_name.\n");
        return EXIT_FAILURE;
    }

    if (strcmp(protocol, "GetFile") != 0) {
        printf("[parse_request] Protocol expected GetFile, received: %s.\n", protocol);
        return EXIT_FAILURE;
    }

    if (strcmp(action, "GET") != 0) {
        printf("[parse_request] Action expected GET, received: %s.\n", action);    
        return EXIT_FAILURE;
    }

    if (strlen(ast->file_name) > MAX_PATH_LEN) {
        printf("[parse_request] Filepath exceeds %d characters.\n", MAX_PATH_LEN);
        return EXIT_FAILURE;
    } 

    /* Once the request is parsed, we can construct the full path to the requested file. We'll 
    store the full path in the file_name field of the asset, ast. First we'll copy workload_dir
    into the filename and then concatenate the filename to it. Note that we explicitly append a 
    slash "/" to the file_name to ensure that we still return the file if the user did not append
    it. According to unix spec, multiple slashes are treated as one. */
    strcpy(ast->file_name, ast->workload_dir);
    strcat(ast->file_name, "/");
    strcat(ast->file_name, file_name);

    return EXIT_SUCCESS;
}

/* check request for proper format 'GetFile GET /path/to/file \0' */
int check_request (char *req) {
    int i;
    
    for (i = 0; i < REQ_SIZE; i++){
        if (req[i]=='\0'){
            return 0;
        }
    }

    return 1;
}

/* send the response header */
int send_response_header(int sock_desc, Asset *rp) {
    long header_size;               /* Size of the response header */
    struct stat st;                 /* Struct to hold file stats */
    char resp_hdr_str[REQ_SIZE];    /* Null terminated string to hold response header */
    char *resp_hdr;                 /* binary response header, null termination removed, because we dont want a null between filesize and beginning of data */                                
    size_t bytes_sent;              /* bytes sent in the current iteration */
    int retval = 0;

    /* Check if file exists and is readble */
    if (access(rp->file_name, R_OK) == 0) {  
        stat(rp->file_name, &st);
        rp->file_size = st.st_size;

        /* Header size is set at the return value of sprintf, which returns the number of chars 
        written EXCLUDING the terminating null. */
        header_size = sprintf(resp_hdr_str, "GetFile OK %ld ", rp->file_size);
    } else { 
        strcpy(resp_hdr_str, "GetFile FILE_NOT_FOUND 0 ");

        /* If file is not found then thats it, we want to send a null terminated response. So the 
        header size, in this case, should include the terminating null */
        header_size = strlen(resp_hdr_str) + 1;
        rp->file_size = 0;
        retval = -2;
    }

    /* construct response header */
    resp_hdr = (char *) malloc(header_size * sizeof(char));

    /* Optionally initialize the memory. Not necessary since string operations will put the 
    necessary '\0' */
    /* memset(resp_hdr, 0 , header_size); */

    /* Construct a response header from resp_hdr_str by removing the terminating null char */
    memcpy(resp_hdr, resp_hdr_str, header_size);

    /* Handle sprintf error */
    if (header_size < 0) {
        printf("[send_response_header] Error constructing response header.\n");
        return -1;
    }

    /* Print header on server side stdout */
    // printf("[send_response_header] response header: %s\n", resp_hdr_str);

    /* Write the header on client socket */
    bytes_sent = send(sock_desc, resp_hdr, header_size, 0);
    if (bytes_sent < 0) {
        fprintf(stderr, "[send_response_header] %s\n", strerror(errno));
        return -1;
    }    
    free(resp_hdr);
    return retval;
}

/* sends the specified file */
int send_file(int socket_desc, Asset *ast){

    FILE *rf;                   /* File descriptor for requested file */
    int bytes_sent, tot_sent;   /* Number of bytes written to output socket */
    char buf[BUF_SIZE];
    int bytes_read;
    

    /* Open the file and check successful opening */
    rf = fopen(ast->file_name, "rb");

    /* Handle file opening error */
    if (rf == NULL) {
        fprintf(stderr, "[send_file][thread_id %d] Error opening file: %s\n", ast->thread_id, ast->file_name);
        return EXIT_FAILURE;
    }

    /* send the requested file */    
    tot_sent = 0;
    while(( bytes_read = fread(buf, 1, BUF_SIZE, rf) ) != 0) {
        bytes_sent = send(socket_desc, buf, bytes_read, 0);
        if (bytes_sent != bytes_read) {
            printf("[send_file][thread_id %d] Partial data sent. bytes_read = %d, bytes_sent = %d\n", ast->thread_id, bytes_read, bytes_sent);
        }
        tot_sent += bytes_sent;
    }

    if (tot_sent != ast->file_size) {
        printf("[send_file][thread_id %d] Warning: %s: File may be sent paritally.\n",
            ast->thread_id, ast->file_name);
    }

    /* Clean up */
    fclose(rf);    
    return EXIT_SUCCESS;
}

/* queue_init */
Queue *queue_init(size_t size) {
    Queue *q;
    q = (Queue *) malloc(sizeof(Queue));

    if (q == NULL) {
        return NULL;
    }

    q->buf = (int *) malloc(size * sizeof(int));

    if (q->buf == NULL) {
        return NULL;
    }

    q->is_empty = 1;
    q->is_full = 0;
    q->head = 0;
    q->tail = 0;

    return q;
}

/* queue_push */
void queue_push(Queue *q, int in) {
    q->buf[q->tail] = in;
    q->tail++;

    if (q->tail == MAX_QUEUE_SIZE) {
        q->tail = 0;
    }

    if (q->tail == q->head) {
        q->is_full = 1;
    }

    q->is_empty = 0;

    return;
}

/* queue_pop */
void queue_pop(Queue *q, int *out) {
    *out = q->buf[q->head];
    q->head++;

    if (q->head == MAX_QUEUE_SIZE) {
        q->head = 0;
    }

    if (q->head == q->tail) {
        q->is_empty = 1;
    }

    q->is_full = 0;

    return;
}

/* queue_free */
void queue_free(Queue *q) {
    free(q->buf);
    free(q);
    return;
}

/* print help message */
void print_help() {
    printf("usage:\n ./webserver [options]\n");
    printf("options:\n");
    
    printf("\t-p [server_port]    Server port (Default: 8888)\n");
    printf("\t-t [thread_count]   Num worker threads (Default: 1, Range: 1-100)\n");
    printf("\t-f [workload_dir]   Path to workload file (Default: . (current directory))\n");
    printf("\t-h                  Show this help message\n");                    
}
