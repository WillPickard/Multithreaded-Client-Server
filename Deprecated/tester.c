#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void){
	printf("top\n");
	FILE *f;

	f = fopen("test.txt", "r+");
	int fileno = fileno;
	
	printf("fileno : %d\n", fileno);

			char path[128] = "/proc/self/fd/";
                        char s[24];
                        sprintf(s, "%d", fileno);
                        printf("s: %s\n", s);
                        strcat(path, s);
                        path[strlen(path)] = '\0';
                        printf("sending: %s\n", path);
                        char buf[128];
                        readlink(path, &buf, sizeof buf);
                        printf("returned: %s\n", buf);
	
	fclose(f); 

}
