#include <stdio.h>

int main(int argc,char **argv) {

    /*array*/    

    /* 1,2,3,4,5,6,7,8,9 */
    

    int size[10];

    size[0]=28;
    size[1]=29;
    size[2]=30;

    for(int i=0;i<10;i++) {
        size[i] = 28 + i;
    }

    for(int i=0;i<10;i++) {
        printf("size : %d\n",size[i]);
    }

    int x=10;
    double dsize[x];

    printf("sizeof dsize = %d\n",sizeof(dsize));
    printf("sizeof dsize[0] = %d\n",sizeof(dsize[0]));

    for(int i=0;i<sizeof(dsize) / sizeof(dsize[0]);i++) {
        dsize[i]=28 + 0.5 * i;
    }
    for(int i=0;i<sizeof(dsize) / sizeof(dsize[0]);i++) {
        printf("dsize : %f\n",dsize[i]);
    }
        
    return 0;
}
