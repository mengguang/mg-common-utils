APR_FLAGS = `pkg-config --cflags --libs apr-1` 
APU_FLAGS = `pkg-config --cflags --libs apr-util-1` 
all: apr_random
apr_random: apr_random.c 
	gcc -std=c99 -o apr_random ${APR_FLAGS} ${APU_FLAGS} apr_random.c
clean:
	rm -f apr_random *.o 
