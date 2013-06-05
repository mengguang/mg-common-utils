APR_FLAGS = `pkg-config --cflags --libs apr-1` 
APU_FLAGS = `pkg-config --cflags --libs apr-util-1` 
all: apr_mmap
apr_mmap: apr_mmap.c 
	gcc -std=c99 -o apr_mmap ${APR_FLAGS} ${APU_FLAGS} apr_mmap.c
clean:
	rm -f apr_mmap *.o 
