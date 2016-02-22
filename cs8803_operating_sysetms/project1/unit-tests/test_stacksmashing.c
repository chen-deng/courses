/* program to cause and cure stack smashing */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define FMT_STR_SZ 25

int main( int argc, char* argv[]) {
	char s1[50], s2[50];
	int s1_len, s2_len;	
	char fmt_str[FMT_STR_SZ];


	if (argc != 4) {
		printf("Usage: ./a.out str1 str1_len str2_len\n");
		return -1;
	}

	s1_len = atoi(argv[2]);
	s2_len = atoi(argv[3]);

	snprintf(fmt_str, FMT_STR_SZ, "%%%ds %%%ds", s1_len, s2_len);
	printf("fmt_str = %s\n", fmt_str);

	sscanf(argv[1], fmt_str, s1, s2);
	printf("s1 = %s, s2 = %s\n", s1, s2);
	return 0;
}