APR_FLAGS = `pkg-config --cflags --libs apr-1` 
APU_FLAGS = `pkg-config --cflags --libs apr-util-1` 
all: apr_str
apr_str: apr_str.c 
	gcc -o apr_str ${APR_FLAGS} ${APU_FLAGS} apr_str.c
clean:
	rm -f apr_str *.o 
