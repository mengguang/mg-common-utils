#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_file_io.h>
#include <apr_file_info.h>
#include <apr_errno.h>

int main(int argc,char **argv) {
    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool,NULL);

    char *name;
    if(argc > 1) {
        name=argv[1];
    } else {
        name="apr_read.c";
    }

    apr_file_t *file;
    apr_status_t st;
    char error[1024];
    st=apr_file_open(&file,name,APR_READ,APR_REG,pool);
    if(st != APR_SUCCESS) {
        fprintf(stderr,"%s\n",apr_strerror(st,error,sizeof(error)));
        return st;
    }
    char buf[1024];
    while(APR_EOF != apr_file_eof(file)) {
        st=apr_file_gets(buf,sizeof(buf),file);
        printf("%s",buf);
    }
    apr_file_close(file);

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

