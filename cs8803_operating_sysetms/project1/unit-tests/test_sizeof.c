/* Program to test sizeof() and strlen behavior */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {
	char str[25] = "hello";
	printf("sizeof(str) = %ld\n", sizeof(str));
	printf("strlen(str) = %ld\n", strlen(str));
	return 0;

}
