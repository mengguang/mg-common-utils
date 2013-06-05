#include <stdio.h>
#include <string.h>

int main(int argc,char **argv) {

	char *name="laomeng";
	char *email="laomeng188@163.com";
	char *blog="http://laomeng188.blog.163.com";
	
	printf("length of name: %d\n",strlen(name));
	printf("sizeof name: %d\n",sizeof(name));
	printf("sizeof char * : %d\n",sizeof(char *));

	for(int i=0;i<strlen(name);i++) {
		printf("%d => %c\n",i,name[i]);
	}

	//name[3]='x';

	return 0;
}
