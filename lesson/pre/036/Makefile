APR_FLAGS = `pkg-config --cflags --libs apr-1`
APU_FLAGS = `pkg-config --cflags --libs apr-util-1`
all: apr_thread_atomic
apr_thread_atomic: apr_thread_atomic.c 
	gcc -std=c99 -o apr_thread_atomic ${APR_FLAGS} ${APU_FLAGS} apr_thread_atomic.c
clean:
	rm -f apr_thread_atomic *.o 
