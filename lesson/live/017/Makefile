APR_FLAGS = `pkg-config --cflags --libs apr-1` 
all: apr_read
apr_read: apr_read.c 
	gcc -o apr_read ${APR_FLAGS} apr_read.c
clean:
	rm -f apr_read *.o
