#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_strings.h>
#include <apr_memcache.h>

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
    
    apr_memcache_t *mc; 
    st=apr_memcache_create(pool,1,0,&mc);
    if(st != APR_SUCCESS) {
        apr_err("apr_memcache_create()",st);
        return st;
    }
    apr_memcache_server_t *server;
    apr_memcache_server_create(pool,"localhost",11211,1,3,5,120,&server);
    apr_memcache_add_server(mc,server);
    
    char *key="laomeng";
    char *value="laomeng188@163.com";
    apr_size_t len=strlen(value) + 1;
    
    st=apr_memcache_set(mc,key,value,len,10,0);
    if(st != APR_SUCCESS) {
        apr_err("apr_memcache_set()",st);
        return st;
    }
    char *data=NULL;
    apr_size_t dsize;
    apr_uint16_t flags;

    for(int i=0;i<20;i++) {
        st=apr_memcache_getp(mc,pool,key,&data,&dsize,&flags);
        if(st != APR_SUCCESS) {
            apr_err("apr_memcache_getp()",st);
            break;
        }
        printf("data : %s\n",data);
        apr_sleep(1000000);
    }

    char *version;
    apr_memcache_version(server,pool,&version);
    printf("version : %s\n",version);
    

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

