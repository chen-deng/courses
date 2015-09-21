## Project 3

### High level description

Project 3 aims to build a client-server system with a
multi-threaded proxy server that is capable of fetching requested resources from the internet (via `curl`) as well as a from a multithreaded cache. 

The flow of a client request is as follows. The client thread makes a resource request to the server via a fictitious protocol called `GetFile`. `GetFile` was implemented in [Project 1](https://github.com/rohan-kekatpure/coursework/tree/master/intro-to-os/project1). The server can satisfy the client request in either of the following three ways:

1. Source the file from its local filesystem
2. Fetch the resource from the internet
3. Query the cache before fetching from the internet

Approach 1 was the subject of [Project1](https://github.com/rohan-kekatpure/coursework/tree/master/intro-to-os/project1). Project 2 focuses on approaches (2) and (3).

[Part 1](https://github.com/rohan-kekatpure/coursework/tree/master/intro-to-os/project3/part1), which implements approach (2) above, enhances the working of the webserver via a multithreaded proxy that allows the server to fetch the requested resource from the internet. [Part 2](https://github.com/rohan-kekatpure/coursework/tree/master/intro-to-os/project3/part2) implements multithreaded caching.