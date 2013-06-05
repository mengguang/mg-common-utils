#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_strings.h>
#include <apr_time.h>
#include <apr_hash.h>

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

    apr_hash_t *ht;
    ht=apr_hash_make(pool);
    char *key;
    char *value;

    key="laomeng";
    value="laomeng188@163.com";
    apr_hash_set(ht,key,strlen(key),value);
    key="laozhang";
    value="laozhang250@163.com";
    apr_hash_set(ht,key,strlen(key),value);
    
    char *getvalue=apr_hash_get(ht,key,strlen(key));
    printf("getvalue : %s\n",getvalue);
    printf("total items: %u\n",apr_hash_count(ht));

    apr_hash_set(ht,key,strlen(key),NULL);
    printf("total items: %u\n",apr_hash_count(ht));

    apr_hash_clear(ht);
    printf("total items: %u\n",apr_hash_count(ht));
    getvalue=apr_hash_get(ht,key,strlen(key));
    printf("getvalue : %s\n",getvalue);

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

