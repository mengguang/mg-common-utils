#include <stdio.h>

int main(int argc,char **argv) {
	char a=60;
	printf("a = %d\n",a);
	short b=100;
	printf("b = %d\n",b);
	int c=200000;
	printf("c = %d\n",c);
	long d=30000000000L;
	printf("d = %ld\n",d);
	float e=3.141592623;
	printf("e = %f\n",e);
	double f=3.1415926939;
	printf("f = %f\n",f);
	long long g= 300000000000000000L;
	printf("g = %lld\n",g);
	
	printf("size of char: %lu\n",sizeof(a));
	printf("size of short: %lu\n",sizeof(b));
	printf("size of int: %lu\n",sizeof(c));
	printf("size of long: %lu\n",sizeof(d));
	printf("size of float: %lu\n",sizeof(e));
	printf("size of double: %lu\n",sizeof(f));
	printf("size of long long: %lu\n",sizeof(g));

	unsigned char ua=250;
	printf("ua = %u\n",ua);
	unsigned short ub=65530;
	printf("ub = %u\n",ub);
	unsigned int uc = 1141100895;
	printf("uc = %u\n",uc);
	unsigned long ud = 1304323289739999L;
	printf("ud = %lu\n",ud);

	printf("size of unsigned char: %lu\n",sizeof(ua));
	printf("size of unsigned short: %lu\n",sizeof(ub));
	printf("size of unsigned int: %lu\n",sizeof(uc));
	printf("size of unsigned long: %lu\n",sizeof(ud));
	return 0;

}

