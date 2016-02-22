#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFSIZE 2

int main(int argc, char *argv[]) {
	char *s = malloc(0);
	// memset(s, 'a', 200);
	strcpy(s, "hello");

	printf("%s\n", s);

	free(s);
	return 0;

}