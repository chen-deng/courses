#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<ctype.h>


int main(int argc, char *argv[]) {

	int c;
	int aflag = 0;
	int avalue = 0;
	int bflag = 0;
	int cflag = 0;
	char *cvalue = NULL;

	/* Parse command line arguments */
	opterr = 0;
	while((c = getopt(argc, argv, ":a:bc:")) != -1) {
		switch (c) {
			case 'a':
				aflag = 1;
				avalue = atoi(optarg);
				break;
			case 'b':
				bflag = 1;
				break;
			case 'c': 
				cflag = 1;
				cvalue = optarg;
				break;
			case '?':
				if (optopt == c){
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				} else if (isprint(optopt)) {
					fprintf(stderr, "Unknown option -%c.\n", optopt);
				} else {
					fprintf(stderr, "Unknown option characer \\x%x.\n", optopt);
				}
				return 1;
			case ':':
				printf("Option %c requires an argument\n", optopt);
				exit(1);
				break;
			default:
				abort();
		}
	}
	printf("aflag = %d, avalue = %d, bflag = %d, cflag = %d, cvalue = %s\n", 
			aflag, avalue, bflag, cflag, cvalue);

	/* spawn threads */
	
	return 0;
}