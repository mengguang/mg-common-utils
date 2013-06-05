#include <stdio.h>
#include <string.h>

int main(int argc,char **argv) {

    char *src;
    char *dst;
    if(argc !=3 ) {
        fprintf(stderr,"usage: copy src dst\n");
        return -1;
    }

    src=argv[1];
    dst=argv[2];

    FILE *srcfp=fopen(src,"r");
    if(!srcfp) {
        perror("src fopen");
        return -2;
    }

    FILE *dstfp=fopen(dst,"w+");
    if(!dstfp) {
        perror("dst fopen");
        fclose(srcfp);
        return -3;
    }

    int size=32;
    char buf[size];
    size_t nread,nwrite;
    
    while(!feof(srcfp)) {
        nread=fread(buf,1,size,srcfp); 
        nwrite=fwrite(buf,1,nread,dstfp);
        printf("read %d bytes, write %d bytes\n",nread,nwrite);
    }
    fclose(srcfp);
    fclose(dstfp);

    return 0;
}

