APR_FLAGS = `pkg-config --cflags --libs apr-1` 
APU_FLAGS = `pkg-config --cflags --libs apr-util-1` 
all: apr_hash
apr_hash: apr_hash.c 
	gcc -std=c99 -o apr_hash ${APR_FLAGS} ${APU_FLAGS} apr_hash.c
clean:
	rm -f apr_hash *.o 
