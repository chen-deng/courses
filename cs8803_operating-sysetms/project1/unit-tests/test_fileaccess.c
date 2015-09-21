/* Program to test file access. Outputs true if the file exists and has read permissions. 

COMPILATION: gcc -Wall test_fileaccess.c -o a.out
USAGE: ./a.out "/path/to/file"
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char *argv[]) {
	// char filename[200];

	if (argc != 2) {
		printf("usage: %s \"filepath/within/quotes\"\n", argv[0]);
		return 1;
	}

	char *filename = argv[1];

	if (access(filename, F_OK) == 0) {
		printf("%s exists.\n", filename);
	}

	if (access(filename, R_OK) == 0) {
		printf("%s exists and is readable.\n", filename);
	}

	if (access(filename, W_OK) == 0) {
		printf("%s exists and is writable.\n", filename);	
	}

	if (access(filename, X_OK) == 0) {
		printf("%s exists and is executable.\n", filename);	
	}

	return 0;

}
