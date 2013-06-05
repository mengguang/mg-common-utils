APR_FLAGS = `pkg-config --cflags --libs apr-1` 
APU_FLAGS = `pkg-config --cflags --libs apr-util-1` 
all: apr_user
apr_user: apr_user.c 
	gcc -std=c99 -o apr_user ${APR_FLAGS} ${APU_FLAGS} apr_user.c
clean:
	rm -f apr_user *.o 
