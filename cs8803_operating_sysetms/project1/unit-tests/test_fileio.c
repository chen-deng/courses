/* Program to understand file IO behavior */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* change buffersize and observe changes in output */
#define BUF_SIZE 500

int main (int argc, char * argv[]) {

	char send_buf[BUF_SIZE];
	int read_size;

	if (argc != 2) {
		printf("usage: %s \"/path/to/file\"\n", argv[0]);
		return EXIT_FAILURE;
	}

	FILE *fp = fopen(argv[1], "rb");

	if (fp == NULL) {
		printf("[main] Error opening file\n");
		return EXIT_FAILURE;
	}

	while (!feof(fp)) {
		/* read BUF_SIZE bytes from the file and print */
		read_size = fread(send_buf, 1, BUF_SIZE, fp);
		puts(send_buf);

		/* Zero out the buffer */
		memset(send_buf, 0, BUF_SIZE);
	}
	return 0;
}