## Part 2 

### High level description 
Part 2 enhances the webserver by allowing it to service client
requests using files cached from previous client requests. 

### Proxy-cache interaction overview

The central aspect of Part 2 of the project is the design and the
implementation of proxy-cache interaction. The webproxy and the cache run as two different long-running (daemon) processes that chare the main memory on the system. The design needs to establish a fast shared-memory data transfer channel between them using Linux interprocess communication (IPC) mechanisms. The design needs to account for multithreading in both the webproxy and the cache and must establish IPC at the individual thread level. For reliable operation in multithreaded environment, the design should also be aware that, in the beginning, the number of cached files could be small leading to resource contention and possible deadlocking of webproxy threads.

The flow of a typical client request is as follows. The client makes a request to the webproxy. Before fetching from the web, the proxy queries the cache daemon. For each request from proxy to cache, the two processes perform a two step communication, with each step having multiple substeps.

1. Header transfer (handshake) 

	1.1 Proxy thread handler queries cache with URL, and shared memory (SHM) id for communication

	1.2 Cache writes the file size to the appropriate field of SHM structure (-1 if file not found)

2. File transfer

	2.2 Proxy thread handler receives file size and sends header to the client 

	2.3 Proxy thread handler becomes ready to receive the file by initializing a pair of semaphores

	2.4 Proxy thread handler and cache thread handler perform data transfer by toggling the semaphore pair (i.e. writer waits until reader signals completion of read and vice versa)

### Proxy-cache interaction details
#### Header transfer (handshake)
We use [POSIX message queues](http://linux.die.net/man/7/mq_overview). Upon receiving the client request, the proxy process initiates the handshake by placing a message on the message queue. Proxy shares three pieces of information in its handshake message: (1) URL of the requested file (2) shared  memory segment id for possible file transfer and (3) size of the shared memory segment. The cache thread is listening to the message queue. Once a message arrives, it parses the message into the path, and the size and the id of the shared memory segment id. It then looks in its repository for the requested file. If the cache does not have the file, it signals a `NOT FOUND` flag in the shared memory structure (described below). The proxy communicates that to the client and this is the end of this request transaction.

If the file is found in the cache, the cache sends the file size information to proxy who in turn passes this information to the client.

#### File transfer and the organization of the shared memory
We use [POSIX shared memory](http://man7.org/linux/man-pages/man7/shm_overview.7.html) for data transfer from cache to the proxy process. 

During its initialization, the webproxy process creates shared memory
segments based on the `-z` (size)  and `-n` (count) command line
arguments and puts them in a FIFO queue. The address of this queue is
passed to each worker thread (`handle_with_cache`). Each shared memory segment is divided into two parts: the SHM metadata portion and the data transfer buffer. The metadata portion has the following
structure:

```c
typedef struct {
    sem_t sem_writer;           /* writer semaphore */
    sem_t sem_reader;           /* reader semaphore */    
    size_t file_size;           /* File size if it exists, -1 if not */
    int cnt;                    /* Indicator for file read complete */
} Shm_Block;
``` 

Size of `Shm_Block` structure is 76 bytes (32 bytes each for the
semaphores, 8 bytes for `file_size`  and 4 bytes for `cnt`). The rest
of the segment is used as a data transfer buffer. To guard against
the pathalogical case of `segment_size < sizeof(Shm_Block)`, we *add*
`sizeof(Shm_block)` to the  requested segment size before allocation.
Linux itself (currently) allocates shared memory in block  sizes of 4
KiB as well.

In its initialization routine, the proxy thread handler function
(`handle_with_cache`) (1) securely pops a shared memory segment from
the FIFO queue and (2) initializes semaphore states of the reader
(lock) the writer (unlocked) before putting the message on the message
queue. Once the cache worker thread reads and parses the message, it
writes the file size to the `file_size` field of `Shm_Block` and
toggles semaphore states.

In the next step, `handle_with_cache` reads the file size, sends the
appropriate header and toggles the semaphore states. If the cache
response was `NOT FOUND`,  the shared memory is securely returned back
to the queue. This completes the header transfer.

Finally, the cache worker thread reads the data from the file and
places it in the shared memory in chunks of `segment_size` and the
reader (`handle_With_cache`) reads it in same chunk size. The  reader
and writer alternate until the file transfer is complete. After the
file transfer, the shared memory segment is securely returned to the
queue.

### Cleanup
The message queue and the shared memory segments are allocated by the
webproxy process and are  cleaned up at its termination. When the
process is terminated with the `Ctrl + C` signal, the  signal handler
cleans up the message queue and the shared memory segments.

The signal handler for the cache daemon simply closes file descriptor
for the message queue.

### Observations
1. Working the `handle_with_cache` depends critically on proper
locking of the FIFO queue and  returning of the SHM segment id back to
the queue. The threads dedlock otherwise
2. Using POSIX semaphore pairs is an effective way to synchronize
access to shared memory. It takes a bit of careful design, but results
in faster data transfer speeds.
3. **Never ignore return value of system calls**. Being disciplined in
checking the return values leads to early discovery of bugs.
4. **Never ignore compiler warning messages**. Harmless compiler
warning are early indicators of  ugly runtime crashes.















