APR_FLAGS = `pkg-config --cflags --libs apr-1` 
APU_FLAGS = `pkg-config --cflags --libs apr-util-1` 
all: apr_base64
apr_base64: apr_base64.c 
	gcc -o apr_base64 ${APR_FLAGS} ${APU_FLAGS} apr_base64.c
clean:
	rm -f apr_base64 *.o
