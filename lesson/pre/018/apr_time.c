#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_time.h>

int main(int argc,char **argv) {
    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool,NULL);


    apr_time_t now=apr_time_now();
    char *buf=apr_pcalloc(pool,1024);
    apr_ctime(buf,now);
    printf("%s\n",buf);

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

