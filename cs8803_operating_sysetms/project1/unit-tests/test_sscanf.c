/* Program to test string teokenization using sscanf. We want to use sscanf to parse a space-separated
request into tokens. 

Usage: ./a.out "string with tokenn"
For ex: ./a.out "GETFile GET /path/to/file"

sscanf will ignore multiple spaces between tokens as well as extra tokens. This is nice. */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {
	char protocol[100], verb[100], filename[200];

	if (argc != 2) {
		printf("usage: %s \"string within quotes\"\n", argv[0]);
		return 1;
	}
	char *request = argv[1];

	sscanf(request, "%s %s %s", protocol, verb, filename);

	printf("protocol = %s, verb = %s, filename = %s\n", protocol, verb, filename);

	if (strcmp(protocol, "GETFile") == 0) {
		printf ("Protocol correct\n");
	}

	if (strcmp(verb, "GET") == 0) {
		printf("Verb correct\n");
	}

	return 0;

}
