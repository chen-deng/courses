#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 10000

void *thread_func(void *arg) {
	int i = *(int *) arg;
	printf("Hello from thread %d\n", i);
	return NULL;
}

int main (int argc, char *argv[]) {
	int i, j, num_threads;

	if (argc < 2) {
		printf("Usage: ./a.out <num_threads>\n");
		exit(1);
	}
	num_threads = atoi(argv[1]);

	pthread_t *threads = (pthread_t *) malloc(num_threads * sizeof(pthread_t));
	int *tnum = (int *) malloc(num_threads * sizeof(int));

	for (i = 0; i < num_threads; ++i ) {
		tnum[i] = i; /* revent race condition for "i" */ 
		pthread_create(&threads[i], NULL, thread_func, (void *) &tnum[i]);
	}

    for (j = 0; j < num_threads; ++j) {
        pthread_join(threads[j], NULL);

    }

    return 0;

}