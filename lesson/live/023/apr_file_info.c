#include <stdio.h>
#include <apr.h>
#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_strings.h>
#include <apr_file_info.h>
#include <apr_time.h>

void apr_err(const char *s, apr_status_t rv)
{
    char buf[120];
    fprintf(stderr, "%s: %s (%d)\n", s, apr_strerror(rv, buf, sizeof buf), rv);
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
        filename="apr_file_info.c";
    }

    apr_finfo_t info;
    st=apr_stat(&info,filename,0,pool);   
    if(st != APR_SUCCESS) {
        apr_err("apr_stat()",st);
        return st;
    }
    printf("size = %d\n",info.size);
    char *stime=apr_pcalloc(pool,1024);
    st=apr_ctime(stime,info.mtime);
    printf("mtime = %s\n",stime);

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}
