APR_FLAGS = `pkg-config --cflags --libs apr-1` 
APU_FLAGS = `pkg-config --cflags --libs apr-util-1` 
all: apr_dbm
apr_dbm: apr_dbm.c 
	gcc -o apr_dbm ${APR_FLAGS} ${APU_FLAGS} apr_dbm.c
clean:
	rm -f apr_dbm *.o friends.*
