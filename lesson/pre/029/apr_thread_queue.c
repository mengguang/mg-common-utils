#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_strings.h>
#include <apr_thread_pool.h>
#include <apr_queue.h>
#include <apr_portable.h>

void apr_err(const char *s, apr_status_t rv)
{
    char buf[120];
    fprintf(stderr, "%s: %s (%d)\n", s, apr_strerror(rv, buf, sizeof buf), rv);
}

void *thread_func(apr_thread_t *thd,void *arg) {
    apr_queue_t *queue=(apr_queue_t *)arg;
    apr_status_t st;
    void *data;
    apr_os_thread_t ost=apr_os_thread_current();
    for(;;) {
        st=apr_queue_pop(queue,&data);
        if(st != APR_SUCCESS) {
            break;
        }
        printf("thread %ld get job : %ld\n",ost,(long)data);
        apr_sleep(1000000);
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
    
    apr_queue_t *queue;
    apr_queue_create(&queue,1024,pool);
    
    for(long i=0;i<15;i++) { 
        apr_thread_pool_push(tpl,thread_func,(void *)queue,APR_THREAD_TASK_PRIORITY_NORMAL,NULL);
        apr_sleep(50000);
    }
    for(long i=0;i<100;i++) {
        apr_queue_push(queue,(void *)i);
        printf("put job: %ld\n",i);
    }
    for(;;) {
        if(apr_queue_size(queue) == 0) {
            break;
        }
        apr_sleep(1000000);
    }
    apr_queue_term(queue);
    apr_thread_pool_destroy(tpl);

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

