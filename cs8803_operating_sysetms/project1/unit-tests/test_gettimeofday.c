/* Program to get time differences between events */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

int main (int argc, char *argv[]) {

	struct timeval t_before, t_after;
	float usec_elapsed;

	gettimeofday(&t_before, NULL);
	usleep(1000);
	gettimeofday(&t_after, NULL);
	usec_elapsed = (t_after.tv_sec - t_before.tv_sec) * 1.0e6 + (t_after.tv_usec - t_before.tv_usec);

	printf("usec_elapsed = %10.0f\n", usec_elapsed);
	return 0;


}
