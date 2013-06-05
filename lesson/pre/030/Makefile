APR_FLAGS = `pkg-config --cflags --libs apr-1` 
APU_FLAGS = `pkg-config --cflags --libs apr-util-1` 
all: apr_uri
apr_uri: apr_uri.c 
	gcc -std=c99 -o apr_uri ${APR_FLAGS} ${APU_FLAGS} apr_uri.c
clean:
	rm -f apr_uri *.o 
