/* Program to test malloc */

#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {


	int const BUF_SIZE = 10;
	int const STR_SIZE = 200;
	int i;

	char **str_array = (char **) malloc(BUF_SIZE * sizeof(char*));

	/* Allocate memory for each string */
	for (i = 0; i < BUF_SIZE; i++) {
		str_array[i] = (char *) malloc(STR_SIZE * sizeof(char));
	}

	/* populate string array with strings */
	for (i = 0; i < BUF_SIZE; i++) {
		sprintf(str_array[i], "hello %d", i);
	}

	/* print string buffer  */
	for (i = 0; i < BUF_SIZE; i++) {
		printf("str_array[%d] = %s\n", i, str_array[i]);
	}

	/* Free the strings in buffer */
	for (i = 0; i < BUF_SIZE; i++) {
		free(str_array[i]);
	}

	/* Free the buffer */
	free(str_array);

	return 0;

}