APR_FLAGS = `pkg-config --cflags --libs apr-1` 
all: apr_time
apr_time: apr_time.c 
	gcc -o apr_time ${APR_FLAGS} apr_time.c
clean:
	rm -f apr_time *.o
