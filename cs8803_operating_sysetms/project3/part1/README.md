## Part 1

### High level description
As mentioned before, the overall aim of this project is to build a multithreaded client-server system. Part 1 enhances the working of the webserver via a multithreaded proxy that allows the server to fetch the requested resource from the internet. Multithreading is implemented via the Boss-Worker design pattern: the `main` function is the boss who receives requests from the webserver and delegates the actual work to worker threads who invoke the handler function `handle_with_curl`. As its name implies, `handle_with_curl` uses the curl library [`libcurl`](http://curl.haxx.se/libcurl/) to do the actual resource fetching over HTTP.

### Working of `handle_with_curl()`
`handle_with_curl` is divided into two logical parts:

1. Fetching the file from the internet using [curl easy API](http://curl.haxx.se/libcurl/c/libcurl-easy.html) of the libcurl library. 
2. Carefully sending the fetched (and possibly binary) data to the client.

Accordingly, the first part of the `handle_with_curl` is simply a wrapper over curl easy API. It sets the options (via `curl_easy_opt`), specifies a callback function to handle the data (`write_callback()`) and performs the actual HTTP GET (`curl_easy_perform`).

The second sends the received bytes, unmodified, to the client. Similar to [Project 1](https://github.com/rohan-kekatpure/coursework/tree/master/intro-to-os/project1), we have to be careful in byte arithmetic if we want to ensure faithful transmission of binary data such as images and videos.