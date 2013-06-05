#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_strings.h>
#include <apr_time.h>
#include <apr_random.h>

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

    unsigned char *buf=apr_pcalloc(pool,16);
    apr_generate_random_bytes(buf,16);
    for(int i=0;i<16;i++) {
        printf("%02x",buf[i]);
    }
    printf("\n"); 

    unsigned int x=0;
    apr_generate_random_bytes((unsigned char *)&x,sizeof(x));
    printf("%u\n",x);

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

