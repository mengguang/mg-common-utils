#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>


int main(int argc,char **argv) {

    char *dirname=".";
    if(argc > 1) {
        dirname=argv[1];
    }
    GDir *d;
    GError *err=NULL;
    //g_clear_error(&err);
    d=g_dir_open(dirname,0,&err);
    if( err != NULL) {
        fprintf(stderr,"g_dir_open: %s\n",err->message);
        return -1;
    }
    const char *name;
    while((name=g_dir_read_name(d)) != NULL) {
        printf("name: %s\n",name);
    }
    g_dir_close(d);

    return 0;
}

