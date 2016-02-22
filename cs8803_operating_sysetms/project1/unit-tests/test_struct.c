/* Program to test struct behavior */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    char protocol[10];
    char action[10];
} Request;

int uses_request(Request *rp) {
	printf("[uses_request] protocol = %s\n", rp->protocol);
	printf("[uses_request] action = %s\n", rp->action);
	return 0;
}

int main (int argc, char *argv[]) {

	Request rv;
	Request rw; 

	// char a[10], b[10];
	// rv.protocol = "hello";
	// rv.action = "PUT";
	// uses_request(&rv);

	sscanf("aloha GET", "%s %s", rw.protocol, rw.action);
	printf("%s, %s\n", rw.protocol, rw.action);
	// uses_request(&rw);

	return 0;


}
