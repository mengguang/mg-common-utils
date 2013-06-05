#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_strings.h>


void apr_err(const char *s, apr_status_t rv)
{
    char buf[120];
    fprintf(stderr, "%s: %s (%d)\n", 
        s, apr_strerror(rv, buf, sizeof buf), rv);
}

apr_status_t str_clean_up(void *s) {
    char *c=s;
    printf("I will clean up the string: %s\n",c);
    return APR_SUCCESS;
}

int main(int argc,char **argv) {
    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool,NULL);
    apr_status_t st;

    apr_pool_t *subpool;
    apr_pool_create(&subpool,pool);
    char *u=apr_pcalloc(subpool,1024);
    strcpy(u,"xiaozhang");
    apr_pool_cleanup_register(subpool,u,str_clean_up,str_clean_up);

    char *s=apr_pcalloc(pool,1024);
    strcpy(s,"laomeng");
    apr_pool_cleanup_register(pool,s,str_clean_up,str_clean_up);
    char *p=apr_pcalloc(pool,1024);
    strcpy(p,"laozhang");
    apr_pool_cleanup_register(pool,p,str_clean_up,str_clean_up);

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

