#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_thread_proc.h>
#include <apr_time.h>

void *thread_func(apr_thread_t *t,void *v) {
    int i;
    for(i=0;i<10;i++) {
        apr_sleep(500000);
        printf("thread %ld => %d\n",(long)v,i);
    }
    return NULL;
}

int main(int argc,char **argv) {

    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool,NULL);
    
    apr_status_t st;
    
    apr_thread_t *t1;
    apr_thread_t *t2;
    apr_thread_t *t3;
    
    apr_thread_create(&t1,NULL,thread_func,(void *)1L,pool);
    apr_thread_create(&t2,NULL,thread_func,(void *)2L,pool);
    apr_thread_create(&t3,NULL,thread_func,(void *)3L,pool);
    apr_thread_join(&st,t1);
    apr_thread_join(&st,t2); 

    apr_thread_join(&st,t3); 

    apr_pool_destroy(pool);
    apr_terminate();

    return 0;

}
