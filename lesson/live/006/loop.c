#include <stdio.h>

/*
 *  n * (n-1) * (n-2) * ... * 1
 */

long factorial(int n) {
	long result=1;
	for( ; n != 1 ; n--) {
		result = result * n;
	}
	return result;
}

int main(int argc,char **argv) {

	int i;
	for(i=0; i < argc; i++) {
		printf("argv %d is %s\n",i,argv[i]);
	}

	i=0;

	while(i < argc) {
		printf("argv %d is %s\n",i,argv[i]);
		i++;
	}

	i=0;
	do {
		printf("argv %d is %s\n",i,argv[i]);
		i++;
	} while( i < argc);

	int n;
	for(n=1; n<= 20; n++) {
		printf(" %d ! = %ld\n",n,factorial(n));
	}

	return 0;
}

