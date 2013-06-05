#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_strings.h>
#include <apr_uuid.h>

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

    apr_uuid_t uuid;
    apr_uuid_get(&uuid);
    char *uuidstr=apr_pcalloc(pool,APR_UUID_FORMATTED_LENGTH+1);
    apr_uuid_format(uuidstr,&uuid);
    printf("new uuid: %s\n",uuidstr);

    apr_uuid_t uuid2;
    apr_uuid_parse(&uuid2,uuidstr);
    printf("%d\n",memcmp(&uuid,&uuid2,sizeof(uuid)));

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

