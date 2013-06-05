#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_base64.h>

int main(int argc,char **argv) {
    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool,NULL);

    char *srcstr;
    if(argc > 1) {
        srcstr=argv[1];
    } else {
        fprintf(stderr,"usgae: apr_base64 str_to_encode\n");
        return -1;
    }

    char *encstr;
    encstr=apr_pcalloc(pool,apr_base64_encode_len(strlen(srcstr)));
    apr_base64_encode(encstr,srcstr,strlen(srcstr));

    printf("%s\n",encstr);

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

