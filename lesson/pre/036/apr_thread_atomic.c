#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_strings.h>
#include <apr_thread_pool.h>
#include <apr_atomic.h>

apr_uint32_t var=0;


void apr_err(const char *s, apr_status_t rv)
{
    char buf[120];
    fprintf(stderr, "%s: %s (%d)\n", s, apr_strerror(rv, buf, sizeof buf), rv);
}

void *thread_func_normal(apr_thread_t *thd,void *arg) {
    for(int j=0;j<10000000;j++) {
        var++;
    }
    return NULL;
}
void *thread_func_atomic(apr_thread_t *thd,void *arg) {
    for(int j=0;j<10000000;j++) {
        apr_atomic_inc32(&var);
    }
    return NULL;
}

apr_thread_mutex_t *mutex;

void *thread_func_mutex(apr_thread_t *thd,void *arg) {
    for(int j=0;j<10000000;j++) {
        apr_thread_mutex_lock(mutex);
        var++;
        apr_thread_mutex_unlock(mutex);
    }
    return NULL;
}

int main(int argc,char **argv) {
    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool,NULL);
    apr_status_t st;

    char *method;
    if(argc > 1) {
        method=argv[1];
    } else {
        method="normal";
    }
    printf("method: %s\n",method);
    
    apr_atomic_init(pool);
    apr_thread_mutex_create(&mutex,APR_THREAD_MUTEX_DEFAULT,pool);
    
    apr_thread_pool_t *tpl;
    st=apr_thread_pool_create(&tpl,8,128,pool);
    apr_size_t n;
    
    for(long i=0;i<30;i++) { 
        if(strcmp(method,"mutex") == 0) {
            apr_thread_pool_push(tpl,thread_func_mutex,(void *)i,APR_THREAD_TASK_PRIORITY_NORMAL,NULL);
        } else if(strcmp(method,"atomic") == 0) {
            apr_thread_pool_push(tpl,thread_func_atomic,(void *)i,APR_THREAD_TASK_PRIORITY_NORMAL,NULL);
        } else {
            apr_thread_pool_push(tpl,thread_func_normal,(void *)i,APR_THREAD_TASK_PRIORITY_NORMAL,NULL);
        }
    }
    for(long i=0;i<20;i++) {
        n=apr_thread_pool_busy_count(tpl);
        printf("busy thread : %d\n",n);
        apr_sleep(1000000);
        if(n == 0) {
            break;
        }
        printf("var = %u\n",var);
    }
    printf("finally, var = %u\n",var);
        
    apr_thread_pool_destroy(tpl);

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

