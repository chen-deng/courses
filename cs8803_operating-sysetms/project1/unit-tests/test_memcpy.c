/* Program to test memcpy */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define HUNDREDMB 104857600
#define BUF_SIZE 512

int main (int argc, char *argv[]) {
	char protocol[100];
	char status[100];
	int file_size;
	char head_buf[512];
	char *data_buf; 
	int i;
	int space_count = 0;
	int data_index = 0; 
	int left_in_hdr = 0;
	int copy_bytes;

	if (argc != 2) {
		printf("usage: %s \"string within quotes\"\n", argv[0]);
		return 1;
	}

	memset(head_buf, 0, BUF_SIZE);
	strcpy(head_buf, argv[1]);
	sscanf(head_buf, "%s %s %d", protocol, status, &file_size);
	printf("protocol = %s, status = %s, file_size = %d\n", protocol, status, file_size);

	if (strcmp(status, "FILE_NOT_FOUND") == 0 && file_size == 0) {
		return 0; 
	}

	if ( (int)file_size < 0) {
		printf("negative file_size received in header\n");
		return 1;
	}
	if (file_size > HUNDREDMB) {
		printf("File too large\n");
		return 1;
	}

	/* Allocate memory for data_buf, and initialize */
	
	data_buf = (char *) malloc(file_size * sizeof(char));

	if (data_buf == NULL) {
		printf("error allocating memory\n");
		return 1;
	} 
	
	memset(data_buf, 0, file_size);

	/* Parse out the header and copy remaining data to data_buf */
	for (i = 0; i < BUF_SIZE; i++) {
		if (head_buf[i] == ' ') {
			++space_count;
		}

		if (space_count == 3) {
			data_index = i + 1;
			printf("data_index = %d\n", data_index);
			break;
		}
	} 

	if ( (i >= BUF_SIZE - 1) || (data_index == 0)) {
		printf("Incorrect header format\n");
		return 1;
	}

	left_in_hdr = (BUF_SIZE - 1) - data_index;

	if (file_size < left_in_hdr) {
		copy_bytes = file_size;
	} else {
		copy_bytes = left_in_hdr;
	}

	printf("copybytes = %d\n", copy_bytes);
	memcpy(data_buf, head_buf + data_index, copy_bytes);
	printf("data: %s\n", data_buf);

	free(data_buf);
	return 0;

}
