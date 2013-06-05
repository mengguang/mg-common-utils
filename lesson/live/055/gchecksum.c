#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>


int main(int argc,char **argv) {

    if(argc !=3) {
         return -1;
    }
    GChecksumType type;
    if(strcmp("md5",argv[1])==0) {
        type=G_CHECKSUM_MD5;
    } else if(strcmp("sha1",argv[1])==0) {
        type=G_CHECKSUM_SHA1;
    } else if(strcmp("sha256",argv[1])==0) {
        type=G_CHECKSUM_SHA256;
    } else {
        type=G_CHECKSUM_MD5;
    }
    char *filename=argv[2];
    FILE *fp=fopen(filename,"r");
    if(!fp) {
        return -2;
    }
    GChecksum *sum=g_checksum_new(type);
    char *buf=g_slice_alloc0(1024);
    size_t n=0;
    while(!feof(fp)) {
        n=fread(buf,1,1024,fp);
        g_checksum_update(sum,buf,n);
    }
    const gchar *csum=g_checksum_get_string(sum);
    printf("%s\n",csum);

    g_checksum_free(sum);
    g_slice_free1(1024,buf);
    fclose(fp);

    return 0;
}

