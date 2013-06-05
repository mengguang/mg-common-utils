#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>


int main(int argc,char **argv) {

    if(argc != 3) {
        fprintf(stderr,"usage: %s type(md5|sha1|sha256) filename\n",argv[0]);
        return -1;
    }

    GChecksumType type;

    if(strcmp("md5",argv[1]) == 0) {
        type=G_CHECKSUM_MD5;
    } else if (strcmp("sha1",argv[1]) == 0) {
        type=G_CHECKSUM_SHA1;
    } else if (strcmp("sha256",argv[1]) == 0) {
        type=G_CHECKSUM_SHA256;
    } else {
        type=G_CHECKSUM_MD5;
    }
        
    char *filename=argv[2];

    gchar *contents;
    gsize length;

    g_file_get_contents(filename,&contents,&length,NULL);
    char *result=g_compute_checksum_for_data(type,contents,length);
    printf("%s\n",result);
    g_free(contents);
    g_free(result);

    GMappedFile *mfile;
    mfile=g_mapped_file_new(filename,FALSE,NULL);
    length=g_mapped_file_get_length(mfile);
    contents=g_mapped_file_get_contents(mfile);
    result=g_compute_checksum_for_data(type,contents,length);
    printf("%s\n",result);
    g_free(result);

    g_mapped_file_unref(mfile);


    return 0;
}

