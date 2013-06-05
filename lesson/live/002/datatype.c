#include <stdio.h>

int main(int argc,char **argv) {
	char a=100;
	short b=30000;
	int c=1141100895;
	long d=10000000000000L;
	long long e = 200000000000L;
	
	printf("a = %d, sizeof = %d\n",a,sizeof(a));	
	printf("b = %d, sizeof = %d\n",b,sizeof(b));	
	printf("c = %d, sizeof = %d\n",c,sizeof(c));	
	printf("d = %ld, sizeof = %d\n",d,sizeof(d));	
	printf("e = %lld, sizeof = %d\n",e,sizeof(e));	

	unsigned int y=4000000000;
	printf("y = %u, sizeof = %d\n",y,sizeof(y));
	

	return 0;
}
