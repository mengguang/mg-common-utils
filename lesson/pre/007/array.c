#include <stdio.h>

int main(int argc,char **argv) {

	int size[10];
	size[0]=28;
	size[1]=29;
	size[3]-30;

	for(int i=0;i<10;i++) {
		size[i]=28+i;
	}
	for(int i=0;i<10;i++) {
		printf("size : %d\n",size[i]);
	}

	double dsize[10];
	for(int i=0;i<10;i++) {
		dsize[i]=28.5+i;
	}
	for(int i=0;i<10;i++) {
		printf("size : %f\n",dsize[i]);
	}
	

	return 0;
}
