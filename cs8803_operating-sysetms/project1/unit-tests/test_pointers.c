/* Program to test pointer behavior */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {
	int a[10];
	int i;

	for (i = 0; i < 10; i++) {
		a[i] = i * i;
	}

	printf("Before: a = %d\n", *a);

	a+= 1;

	printf("Before: a = %d\n", *a);


	return 0;

}