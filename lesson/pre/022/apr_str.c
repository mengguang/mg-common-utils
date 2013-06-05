#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_strings.h>


void apr_err(const char *s, apr_status_t rv)
{
    char buf[120];
    fprintf(stderr, "%s: %s (%d)\n", s, apr_strerror(rv, buf, sizeof buf), rv);
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

    apr_int64_t i;
    i=apr_atoi64("11411008953232344");
    printf("i  = %ld\n",i);
    char *si=apr_ltoa(pool,i);
    printf("si = %s\n",si);

    char *s=apr_pcalloc(pool,1024);
    strcpy(s,"ni hao are you !");
    apr_collapse_spaces(s,s);
    printf("s = %s\n",s);

    apr_cpystrn(s,"how are you today?",1024);
    printf("s = %s\n",s);
    apr_collapse_spaces(s,s);
    printf("s = %s\n",s);
    
    char *k=apr_psprintf(pool,"%d + %d = %d",100,200,100+200);
    printf("k : %s\n",k);

    char *j=apr_pstrdup(pool,"laoliu");
    printf("j = %s\n",j);

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

