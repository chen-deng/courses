/*
Program to implement and test feeding data from a file into
a round robin queue
*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define MAXSTRLEN 200
/* Round robin queue */
typedef struct {
    char **buf; 			/* buffer to hold array of strings */
    int head;				/* pointer to track writes */
    int tail;				/* pointer to track reads */
    int size;
} RRQueue;

RRQueue *rrqueue_init(int size); 
void rrqueue_pop(RRQueue *q, char *elem);
void rrqueue_push(RRQueue *q, char *elem);
void rrqueue_free(RRQueue *q);
int file_to_queue(FILE *fp, RRQueue *qp);

int main (int argc, char *argv[]) {
	int i;
	int status;
	char spop[MAXSTRLEN];

	RRQueue *qp = rrqueue_init(5);
	FILE *fp = fopen("resources/filewlines.txt", "r");
	
	status = file_to_queue(fp, qp);
	if (status == EXIT_FAILURE) {
		printf("[main] file_to_queue operation failed\n");
		return EXIT_FAILURE;
	}

	for (i = 0; i < 3 * qp->size; i++) {
		rrqueue_pop(qp, spop);
		printf("%s\n", spop);
	}
	return 0;
}

/* Read lines from file and push it to a round-robin queue */
int file_to_queue(FILE *fp, RRQueue *qp) {

	char *line = NULL;
	size_t len = 0;
	ssize_t nchars;

	if (fp == NULL) 
		return EXIT_FAILURE;

	while ((nchars = getline(&line, &len, fp)) != -1) {
		/* ignore empty lines */
		if (nchars == 1)
			continue;

		/* strip newline at end */
		if (line[nchars - 1] == '\n') {
			line[nchars - 1] = '\0';
			--nchars;
		}
		rrqueue_push(qp, line);
	}	
	return EXIT_SUCCESS;
}

/* initialization of RRQueue */
RRQueue *rrqueue_init(int size) {
	int i; 

	RRQueue *q = (RRQueue *) malloc(sizeof(RRQueue));
	q->head = 0;
	q->tail = 0;
	q->size = size; 

	/* allocate the buffer */
	q->buf = (char **) malloc(size * sizeof(char *));

	/* allocate space for each string in the buffer */
	for (i = 0; i < q->size; i++){
		q->buf[i] = (char *) malloc(MAXSTRLEN * sizeof(char));
	}
	return q;
}

/* popping an element */
void rrqueue_pop(RRQueue *q, char *elem) {
	strcpy(elem, q->buf[q->tail]);
	q->tail++;

	/* Wrap around if at the end */
	if (q->tail == q->size) 
		q->tail = 0;
	return;
}

/* push an element */
void rrqueue_push(RRQueue *q, char *elem) {
	/* ignore elem if the queue is full */
	if (q->head == q->size) 
		return; 

	strcpy(q->buf[q->head], elem);
	q->head++;
	return;
}

/* free the queue */
void rrqueue_free(RRQueue *q) {
	int i;
	for (i = 0; i < q->size; i++){
		free(q->buf[i]);
	}
	free(q->buf);
 	free(q);
}























