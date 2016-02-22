/* program to test if we get different pointer for each invocation of a malloc in a loop */
/* Answer: yes, provided the memory isnt freed in the same loop, no otherwise */
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    int i = 0;
    int *p;
    for (i = 0; i < 50; ++i) {
        p = (int *) malloc(20 * sizeof(int));

        printf("value of pointer =%p\n", p);

        //free(p);
    }

    return 0;
}