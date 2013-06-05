#include <stdio.h>

/*
 * k * (k-1) * (k-2) ... * 1
 */

int factorial(int k) {
	int result=1;
	for( ; k>0 ;k-- ) {
		result *= k;
	}
	return result;
}

int factorial2(int k) {

	int result =1;
	while(k > 0) {
		result *= k;
		k--;
	}

}

int main(int argc,char **argv) {

	int i;
	for(i=0;i < argc; i++) {
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

	} while(i < argc);
	
	i=5;
	printf("factorial of is %d\n",factorial(i));
	printf("factorial of is %d\n",factorial2(i));
	
	return 0;
}
