APR_FLAGS = `pkg-config --cflags --libs apr-1`
APU_FLAGS = `pkg-config --cflags --libs apr-util-1`
all: apr_thread_queue
apr_thread_queue: apr_thread_queue.c 
	gcc -std=c99 -o apr_thread_queue ${APR_FLAGS} ${APU_FLAGS} apr_thread_queue.c
clean:
	rm -f apr_thread_queue *.o 
