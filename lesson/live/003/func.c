#include <stdio.h>

void hello() {
	printf("Hello World\n");
	return;
}

int sum(int a,int b) {
	int c = a+b;
	return c;
}

int main(int argc,char **argv) {

	hello();

	int a=100;
	int b=200;
	int c=sum(a,b);
	printf("c = %d\n",c);
	return 0;

}


