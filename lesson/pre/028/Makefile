APR_FLAGS = `pkg-config --cflags --libs apr-1` 
APU_FLAGS = `pkg-config --cflags --libs apr-util-1` 
all: apr_memcache
apr_memcache: apr_memcache.c 
	gcc -std=c99 -o apr_memcache ${APR_FLAGS} ${APU_FLAGS} apr_memcache.c
clean:
	rm -f apr_memcache *.o 
