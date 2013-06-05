APR_FLAGS = `pkg-config --cflags --libs apr-1` 
APU_FLAGS = `pkg-config --cflags --libs apr-util-1` 
all: apr_pool
apr_pool: apr_pool.c 
	gcc -o apr_pool ${APR_FLAGS} ${APU_FLAGS} apr_pool.c
clean:
	rm -f apr_pool *.o 
