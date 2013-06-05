#include <apr.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <stdio.h>

int main(int argc,char **argv) {
    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool,NULL);

    char *str=apr_itoa(pool,1141100895);
    printf("%s\n",str);

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

