#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_strings.h>
#include <apr_uri.h>

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

    apr_uri_t uri;
    char *str="http://www.okbuy.com:80/list?category=166&ref=i_nav";    
    st=apr_uri_parse(pool,str,&uri);
    if(st != APR_SUCCESS) {
        apr_err("apr_uri_parse()",st);
        return st;
    }
    printf("scheme : %s\n",uri.scheme);
    printf("hostinfo : %s\n",uri.hostinfo);
    printf("hostname : %s\n",uri.hostname);
    printf("path : %s\n",uri.path);
    printf("port_str : %s\n",uri.port_str);
    printf("query : %s\n",uri.query);

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

