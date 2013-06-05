APR_FLAGS = `pkg-config --cflags --libs apr-1` 
APU_FLAGS = `pkg-config --cflags --libs apr-util-1` 
all: apr_md5
apr_md5: apr_md5.c 
	gcc -std=c99 -o apr_md5 ${APR_FLAGS} ${APU_FLAGS} apr_md5.c
clean:
	rm -f apr_md5 *.o 
