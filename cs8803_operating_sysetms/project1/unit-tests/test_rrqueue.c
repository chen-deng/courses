/* Program to implement and test a round robin queue */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define STR_SIZE 200
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


int main (int argc, char *argv[]) {
	int i; 
	char str[200], spop[200];
	RRQueue *qp = rrqueue_init(5);

	for (i = 0; i < qp->size; i++) {
		sprintf(str, "hello %d", i);
		rrqueue_push(qp, str);
	}

	for (i = 0; i < 3 * qp->size; i++) {
		rrqueue_pop(qp, spop);		
		printf("%s\n", spop);
	}
	rrqueue_free(qp);
	return 0;
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
		q->buf[i] = (char *) malloc(STR_SIZE * sizeof(char));
	}
	return q;
}

/* popping an element */
void rrqueue_pop(RRQueue *q, char *elem) {
	strcpy(elem, q->buf[q->tail]);
	q->tail++;

	/* Wrap around if at the end */
	if (q->tail == q->size) {
	
		q->tail = 0;
	}
	return;
}

/* push an element */
void rrqueue_push(RRQueue *q, char *elem) {
	/* ignore elem if the queue is full */
	if (q->head == q->size) {
		return; 
	}

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























