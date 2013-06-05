APR_FLAGS = `pkg-config --cflags --libs apr-1` 
APU_FLAGS = `pkg-config --cflags --libs apr-util-1` 
all: apr_signal
apr_signal: apr_signal.c 
	gcc -std=c99 -o apr_signal ${APR_FLAGS} ${APU_FLAGS} apr_signal.c
clean:
	rm -f apr_signal *.o 
