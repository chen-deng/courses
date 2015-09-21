## Project 1: Multithreaded client server system

### Project description and high-level design

The aim of this project is to understand concurrent programming in
Linux using POSIX threads by  implementing a multithreaded file server
and a multithreaded, load-generating client.

The client and the server both follow the boss-worker multi-threading
pattern. The boss-worker pattern  works as follows. The server boss
thread spawns worker threads and listens on a specified port. The boss
thread is light-weight.  When a new client request comes in, the boss
thread simply places it in a FIFO queue. The worker threads pick
(consume) and service the client requests from the queue. Servicing
the requests involves request parsing, sending the response header
with file size, reading the file from disk and sending it over
network.

The client is a load generator for the server. The client boss thread
does two things.  It first reads the workoad file names (i.e. the
files to be requested from the server) and places the filenames  in a
round-robin queue. It then spawns specified number of worker threads
and assignes each thread a number of requests to send to the server.
For each request, the client worker threads pick a filename from the
round-robin queue, open a socket to the server and send request data
on  that socket. The client thread then first receives the response
header and then the data. The clients can optionally write the
received file to the disk.

### The `GetFile` protocol

The client and the server threads communicate via the (fictitious)
`GetFile` protocol.  The server expects the client request in the
following format:

```GetFile GET <file url>```

If the server finds the file, it sends the header

```GetFile OK <byte stream>```

Where `<byte stream>` is the file data. If the requested file is not
found the server sends

```GetFile FILE_NOT_FOUND 0```

Since the aim is to understand concurrent programming, the full HTTP
protocol was not implemented.

### BUILDING THE PROJECT

The project folder includes a `makefile` at the top level. Type

```
make all 			# Makes webclient and webserver
make webclient 		# Makes the webclient
make webserver		# Makes the webserver
```

respectively to make everything, webclient and webserver. The
binary executables are placed in  the `bin` subdirectory of the
project folder. 

### RUNNING THE PROJECT

The project can be run by invoking the webserver and webclient
binaries from the bin  subdirectory. For the webserver, from the
project top level type:

```
bin/webserver -p 8888 -t 1000 -f ./resources
````

This will start the server on port 8888, with 1000 threads, and with
`./resources` as the directory from which static files will be served.
If any options are not provided, defaults values consistent with
project description are assumed. The help message that details the
various options and default option values can be obtained by typing

```
bin/webserver -h
```

For the webclient, from the project top level type any of the
following:

```
bin/webclient -s localhost -p 8888 -t 100 -r 1000 -d ./downloads
bin/webclient -s 0.0.0.0 -p 8888 -t 100 -r 1000 -d ./downloads
bin/webclient -s 127.0.0.1 -p 8888 -t 100 -r 1000 -d ./downloads
```

to run a webclient application which connects to the server at host
`ip = 127.0.0.1`, `port = 8888` with 100 worker threads, to issue 1000
requests and to place downloaded files in `./downloads` directory. If
any options are not provided, defaults values are assumed. The help
message that details the various options and default option values can
be obtained by typing

```
 bin/webclient -h
```

### FILE DESCRIPTIONS

1. webclient.c: Source code for webclient

2. webserver.c: Source code for webserver

3. makefile: Build script

4. bin directory: directory for storing the client and server
executables

5. verifyhash.sh: A little shell utility to compare `MD5` or `SHA1`
checksums of the requested and received files to check the fidelity of
the transmission. The user needs to edit the script and provide
source and download folders. The script then computes the checksums of
all files in the downloads folder with the corresponding filenames
in the source folder. For example, if the server is serving static
files from `./resources` and the client is downloading then to
`./downloads`, then these two locations need to be provided by editing
the script (yes, it'd be nice to make them cmd line args.)

The source codes for webclient and webserver are over 500 lines of
code each. It would be ideal to split them into logically separated
modules. However, I've chosen to keep all code in a single file for
ease of reference. It was a trade-off between long term
maintainability (which demands splitting) vs ease of reading by
keeping everything in the same file. However, I've attempted to
provide ample inline documentation to help understand flow of the
logic.

### NOTES

Sometimes, with high server load, the output of the server and client on stdout seems to be
frozen, but the transfer resumes after a few seconds. Please dont kill the run if you encounter
this situation.



