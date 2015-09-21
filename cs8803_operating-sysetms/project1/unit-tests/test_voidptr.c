/* Program to test behavior of void* pointer */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int i;
} My_struct;

void *process_by_ref(void *x) {
	My_struct *y = (My_struct *) x;
	printf("[process_by_ref] In function: y->i = %d\n", y->i);
	printf("[process_by_ref] Now changing\n");
	y->i = 20;

	return NULL;
}

/**
 * This does not work. Uncommenting the following code results in compilation error 
 * because of the line 
 * My_struct y = (My_struct) x;
 * because we're requesting conversion to non-scalar type.
 */
/*
void *process_by_val(void *x) {
	My_struct y = (My_struct) x;
	printf("[process_by_val] In function: y.i = %d\n", y.i);
	printf("[process_by_val] Now changing\n");
	y.i = 20;

	return NULL;	
}
*/
int main (int argc, char *argv[]) {
	My_struct s;
	
	/* Test process by reference */
	s.i = 10;
	printf("[main] Before function: s.i = %d\n", s.i);
	process_by_ref((void*) &s);
	printf("[main] After function: s.i = %d\n", s.i);

	/* Test proces by value */
	/* Uncommenting will result in compilation errors */
	/*
	s.i = 10;
	printf("[main] Before function: s.i = %d\n", s.i);
	process_by_val((void*) &s);
	printf("[main] After function: s.i = %d\n", s.i);	
	*/
	return 0;


}
