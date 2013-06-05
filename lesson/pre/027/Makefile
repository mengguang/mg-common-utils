APR_FLAGS = `pkg-config --cflags --libs apr-1` 
APU_FLAGS = `pkg-config --cflags --libs apr-util-1` 
all: apr_uuid
apr_uuid: apr_uuid.c 
	gcc -o apr_uuid ${APR_FLAGS} ${APU_FLAGS} apr_uuid.c
clean:
	rm -f apr_uuid *.o 
