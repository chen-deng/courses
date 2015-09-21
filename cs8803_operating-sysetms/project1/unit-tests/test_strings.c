#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {

	char s[] = "GetFile GET /path/to/file";
	int i;
	char protocol[100], action[100], filename[200];

	printf("length = %zu\n", strlen(s));
	printf("%c = %d\n", s[strlen(s)], s[strlen(s)]);

	char *req = (char *) malloc(50 * sizeof(char));

	memset(req, 30, 50);

	strcpy(req, s);

	printf("%s, strlen = %zu\n", req, strlen(req));

	sscanf(req, "%s %s %s", protocol, action, filename);

	printf("p = %s, a = %s, f = %s\n",protocol, action, filename );

	// for (i = 0; i<50; i++) {
	// 	printf("%c = %d\n", req[i],req[i]);
	// }	


	return 0;
}
