#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_strings.h>
#include <apr_thread_pool.h>

void apr_err(const char *s, apr_status_t rv)
{
    char buf[120];
    fprintf(stderr, "%s: %s (%d)\n", s, apr_strerror(rv, buf, sizeof buf), rv);
}

void *thread_func(apr_thread_t *thd,void *arg) {

    long i=(long)arg;
    for(int j=0;j<20;j++) {
        printf("task %d => %d\n",i,j);
        apr_sleep(300000);
    }
    return NULL;
}

int main(int argc,char **argv) {
    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool,NULL);
    apr_status_t st;
    
    apr_thread_pool_t *tpl;
    st=apr_thread_pool_create(&tpl,8,128,pool);
    apr_size_t n;
    
    for(long i=0;i<30;i++) { 
        apr_thread_pool_push(tpl,thread_func,(void *)i,APR_THREAD_TASK_PRIORITY_NORMAL,NULL);
        apr_sleep(50000);
    }
    for(long i=0;i<20;i++) {
        n=apr_thread_pool_idle_count(tpl);
        printf("idle thread : %d\t",n);
        n=apr_thread_pool_busy_count(tpl);
        printf("busy thread : %d\t",n);
        n=apr_thread_pool_threads_count(tpl);
        printf("total thread : %d\n",n);
        apr_sleep(1000000);
    }
        
    apr_thread_pool_destroy(tpl);

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

