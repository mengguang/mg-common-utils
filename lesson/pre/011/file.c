#include <stdio.h>
#include <string.h>

int main(int argc,char **argv) {

    char *file;
    if(argc > 1) {
        file=argv[1];
    } else {
        file="file.c";
    }
    FILE *fp=fopen(file,"r");
    if(!fp) {
        perror("fopen:");
        return -1;
    }
    char line[1024];
    while(!feof(fp)) {
        memset(line,0,1024);
        fgets(line,1024,fp);
        printf("%s",line);
    }
    fclose(fp);
    return 0;
}

