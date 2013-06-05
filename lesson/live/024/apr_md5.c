#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_strings.h>
#include <apr_md5.h>
#include <apr_file_io.h>

void apr_err(const char *s, apr_status_t rv)
{
    char buf[120];
    fprintf(stderr, "%s: %s (%d)\n", 
            s, apr_strerror(rv, buf, sizeof buf), rv);
}

int main(int argc,char **argv) {
    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool,NULL);
    apr_status_t st;

    char *filename;
    if(argc > 1) {
        filename=argv[1];
    } else {
        filename="apr_md5.c";
    }

    apr_md5_ctx_t md5;
    apr_md5_init(&md5);

    apr_file_t *file;
    st=apr_file_open(&file,filename,APR_READ,APR_REG,pool);
    if(st != APR_SUCCESS) {
        apr_err("apr_file_open()",st);
        return st;
    }
    apr_size_t n=1024;
    char *buf=apr_pcalloc(pool,n);
    while(APR_EOF != apr_file_eof(file)) {
        n=1024;
        st=apr_file_read(file,buf,&n);
        apr_md5_update(&md5,buf,n);
    }
    unsigned char result[APR_MD5_DIGESTSIZE];
    apr_md5_final(result,&md5);
    for(int i=0;i<APR_MD5_DIGESTSIZE;i++) {
        printf("%02x",result[i]);
    }
    printf("\n");

    apr_file_close(file);

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

