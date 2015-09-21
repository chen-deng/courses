/* Program to test basename */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libgen.h>

int main (int argc, char *argv[]) {

	char new_file_name[100];

	if (argc != 2) {
		printf("Usage: ./a.out \"/full/path/to/file\"\n");
		return 1;
	}

	// printf("Dirname = %s\n", dirname(argv[1]));
	printf("Filename - only = %s\n", basename(argv[1]));

	strcpy(new_file_name, "/my/new/path//");
	strcat(new_file_name, basename(argv[1]));
	printf("Filename with new path = %s\n", new_file_name);
	return 0;


}
