/* Program to test strcat  */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {
	char left[215] = "GETFile GET ";
	char right[] = "/path/to/file";
	strcat(left, right);
	printf("%s\n", left);

	return 0;


}
