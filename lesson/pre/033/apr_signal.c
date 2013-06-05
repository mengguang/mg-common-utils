#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_strings.h>
#include <apr_signal.h>
#include <apr_time.h>

void apr_err(const char *s, apr_status_t rv)
{
    char buf[120];
    fprintf(stderr, "%s: %s (%d)\n", s, apr_strerror(rv, buf, sizeof buf), rv);
}

void signal_func(int signal) {
    printf("got signal: %d, %s \n",signal,apr_signal_description_get(signal));
}

int main(int argc,char **argv) {
    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool,NULL);
    apr_status_t st;

    apr_signal_init(pool);
    apr_signal(SIGINT,signal_func);
    apr_signal_block(SIGINT);
    printf("signal SIGINT blocked\n");
    apr_sleep(5000000);
    apr_signal_unblock(SIGINT);
    printf("signal SIGINT unblocked\n");
    apr_sleep(50000000);
    

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

