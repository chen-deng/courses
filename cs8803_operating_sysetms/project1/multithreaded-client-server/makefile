CFLAGS=-Wall -pthread
all: webserver webclient

webserver: 
	gcc $(CFLAGS) webserver.c -o bin/webserver

webclient:
	gcc $(CFLAGS) webclient.c -o bin/webclient

clean:
	rm bin/*
