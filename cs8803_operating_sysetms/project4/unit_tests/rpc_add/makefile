# Compiler flags 

CC = gcc
CFLAGS += -g 
LDLIBS += -lnsl
RPCGENFLAGS = -C

# Targets 

all: add_client add_server

# Executables

add_client: add_client.o add_clnt.o add_xdr.o
	$(CC) -o add_client add_client.o add_clnt.o add_xdr.o -lnsl

add_server: add_server.o add_svc.o add_xdr.o
	$(CC) -o add_server add_Server.o add_svc.o add_xdr.o -lnsl

# Object files for executables

add_server.o: add_server.c add.h
	$(CC) $(CFLAGS) -c add_server.c

add_client.o: add_client.c add.h
	$(CC) $(CFLAGS) -c add_client.c 

#Compile files generated by rpcgen

add_svc.o: add_svc.c add.h
	$(CC) $(CFLAGS) -c add_svc.c

add_clnt.o: add_clnt.c add.h
	$(CC) $(CFLAGS) -c add_clnt.c 

add_xdr.o: add_xdr.c add.h
	$(CC) $(CFLAGS) -c add_xdr.c

add_clnt.c add_svc.c add_xdr.c add.h: add.x
	rpcgen $(RPCGENFLAGS) add.x

clean:
	rm -vf code add_client add_server *.o

