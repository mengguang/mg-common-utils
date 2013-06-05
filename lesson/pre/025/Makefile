APR_FLAGS = -DLINUX=2 -D_REENTRANT -D_GNU_SOURCE -pthread -I/usr/include/apr-1  -lapr-1 -lpthread
APU_FLAGS = -DLINUX=2 -D_REENTRANT -D_GNU_SOURCE -pthread -I/usr/include/apr-1  -laprutil-1 -lldap -llber -lexpat -ldb-4.7 -lapr-1 -lpthread 
all: apr_thread_pool
apr_thread_pool: apr_thread_pool.c 
	gcc -std=c99 -o apr_thread_pool ${APR_FLAGS} ${APU_FLAGS} apr_thread_pool.c
clean:
	rm -f apr_thread_pool *.o 
