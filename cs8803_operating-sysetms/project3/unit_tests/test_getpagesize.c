/*
Code to get virtual memory page size
 */

#include <unistd.h>
#include <stdio.h>

int main (int argc, char *argv[]) {
	long sz = sysconf(_SC_PAGESIZE);
	printf("[main] system page size = %ld bytes.\n", sz);
	return 0;
}