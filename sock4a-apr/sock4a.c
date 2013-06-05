#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_file_io.h>
#include <apr_file_info.h>
#include <apr_errno.h>
#include <apr_thread_proc.h>
#include <apr_poll.h>

void apr_err(const char *s, apr_status_t rv)
{
    char buf[120];

    fprintf(stderr,
        "%s: %s (%d)\n",
        s, apr_strerror(rv, buf, sizeof buf), rv);
}

void simpleEncode(char *buf,int len) {
    int i;
    for(i=0;i<len;i++) {
        buf[i] = buf[i]+3;
    }
    return;
}

void simpleDecode(char *buf,int len) {
    int i;
    for(i=0;i<len;i++) {
        buf[i] = buf[i]-3;
    }
    return;
}


void *browser_thread_func(apr_thread_t *th,void *arg) {
    apr_status_t st;
    apr_pool_t *pool=apr_thread_pool_get(th);
    apr_socket_t *browser=(apr_socket_t *)arg;
    apr_socket_t *socks;
    st=apr_socket_create(&socks,APR_INET,SOCK_STREAM,APR_PROTO_TCP,pool);
    apr_sockaddr_t *sa;
    apr_sockaddr_info_get(&sa,"106.187.41.101",APR_INET,21080,0,pool);
    st=apr_socket_connect(socks,sa);
    if(st != APR_SUCCESS) {
        apr_err("apr_socket_connect()",st);
        apr_socket_close(browser);
        apr_pool_destroy(pool);
        return NULL;
    }

    apr_pollset_t *pl;
    st=apr_pollset_create(&pl,2,pool,0);

    apr_pollfd_t pbrowser;
    apr_pollfd_t psocks;
    const apr_pollfd_t *out;

    for(;;) {
        pbrowser.desc_type = APR_POLL_SOCKET;
        pbrowser.desc.s = browser;
        pbrowser.reqevents = APR_POLLIN;
        pbrowser.client_data=(void *)8L;

        psocks.desc_type = APR_POLL_SOCKET;
        psocks.desc.s = socks;
        psocks.reqevents = APR_POLLIN;
        psocks.client_data=(void *)9L;

        apr_pollset_add(pl,&pbrowser);
        apr_pollset_add(pl,&psocks);
        int num; 
        st=apr_pollset_poll(pl,-1,&num,&out);
        apr_size_t n=1024;
        char buf[n];
        if(out[0].client_data == (void *)8L) {
            n=1024;
            st=apr_socket_recv(browser,buf,&n);
            if(n <= 0 || st == APR_EOF) {
                apr_socket_close(socks);
                apr_socket_close(browser);
                apr_pool_destroy(pool); 
                return NULL;
            }
            simpleEncode(buf,n);
            st=apr_socket_send(socks,buf,&n);
            if( n <= 0 || st != APR_SUCCESS) {
                apr_socket_close(socks);
                apr_socket_close(browser);
                apr_pool_destroy(pool); 
                return NULL;
            }
        } else {
            n=1024;
            st=apr_socket_recv(socks,buf,&n);
            if(n <= 0 || st == APR_EOF) {
                apr_socket_close(socks);
                apr_socket_close(browser);
                apr_pool_destroy(pool); 
                return NULL;
            }
            simpleDecode(buf,n);
            st=apr_socket_send(browser,buf,&n);
            if( n <= 0 || st != APR_SUCCESS) {
                apr_socket_close(socks);
                apr_socket_close(browser);
                apr_pool_destroy(pool); 
                return NULL;
            }

        }
        
    }
    apr_socket_close(socks);
    apr_socket_close(browser);
    apr_pool_destroy(pool); 
    return NULL;
}

int main(int argc,char **argv) {
    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool,NULL);
    
    apr_status_t st;
    apr_socket_t *server;
    st=apr_socket_create(&server,APR_INET,SOCK_STREAM,APR_PROTO_TCP,pool);
    apr_socket_opt_set(server,APR_SO_REUSEADDR,1); 
    apr_sockaddr_t *sa;
    apr_sockaddr_info_get(&sa,NULL,APR_INET,21080,0,pool);
    st=apr_socket_bind(server,sa);
    if(st != APR_SUCCESS) {
        apr_err("apr_socket_bind()",st);
        return st;
    }
    
    st=apr_socket_listen(server,128);
    if(st != APR_SUCCESS) {
        apr_err("apr_socket_listen()",st);
        return st;
    }
    for(;;) {
        apr_pool_t *cpool;
        apr_pool_create(&cpool,pool);
        apr_socket_t *browser;
        st=apr_socket_accept(&browser,server,cpool);
        apr_threadattr_t *attr;
        apr_threadattr_create(&attr,cpool);
        apr_threadattr_detach_set(attr,1);
        apr_thread_t *bth;
        apr_thread_create(&bth,attr,browser_thread_func,(void *)browser,cpool);
    }
        
    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

