/* Program to test sprintf */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {
	char str[25];
	int num_chars;

	

	num_chars = sprintf(str, "%s", "hello");
	printf("number of characters written = %d\n", num_chars);
	printf("length of string = %zu\n", strlen(str));

	return 0;

}
