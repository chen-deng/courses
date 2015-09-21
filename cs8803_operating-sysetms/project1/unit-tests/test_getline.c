/* Implements reading lines from a file and placing them into a roundrobin queue */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

    char *line = NULL;
    size_t len = 0;
    ssize_t chars;

    FILE *fp = fopen("./resources/filewlines.txt", "r");

    if (fp == NULL) {
        return EXIT_FAILURE;
    }

    while ((chars = getline(&line, &len, fp)) != -1) {

        /* ignore empty lines */
        if (chars == 1) 
            continue;

        /* strip newlines at end */
        if (line[chars - 1] == '\n') {
            line[chars - 1] = '\0';
            --chars;
        }

        // printf("retrived line of length %zu\n", chars);
        printf("%s\n", line);
    }

    return 0;

}






















