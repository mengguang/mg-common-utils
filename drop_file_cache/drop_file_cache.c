#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#define _XOPEN_SOURCE 600
#include <fcntl.h>

int main(int argc,char **argv)
{
    if(argc != 2) {
        printf("Usage: %s file_to_drop\n",argv[0]);
        return 1;
    }
    char *name=argv[1];
    FILE *fp=fopen(name,"r");
    if(!fp){
        perror("open file error:");
        return -1;
    }
    if(fdatasync(fileno(fp))==-1) {
        perror("fdatasync failed:");
    }
    if(posix_fadvise(fileno(fp),0,0,POSIX_FADV_DONTNEED) == -1){
        perror("posix_fadvise failed:");
    }
    fclose(fp);
    return 0;
}
