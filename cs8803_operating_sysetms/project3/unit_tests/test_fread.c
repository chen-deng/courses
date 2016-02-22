#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#define BUF_SIZE 256

int main(int argc, char *argv[]) {

	if (argc != 2) {
		printf("[main] Usage: ./a.out <filename>\n");
		exit(1);
	}

	int fd;
	char buf[BUF_SIZE];

	fd = open(argv[1], O_RDONLY);
	int bytes_read = 0;

    while ((bytes_read = read(fd, buf, BUF_SIZE)) != 0) {
    	printf("\n[main] bytes_read = %d\n", bytes_read);
    	write(STDOUT_FILENO, buf, bytes_read);
    } 

   	return 0;
}