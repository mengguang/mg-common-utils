APR_FLAGS = `pkg-config --cflags --libs apr-1` 
APU_FLAGS = `pkg-config --cflags --libs apr-util-1` 
all: apr_file_info
apr_file_info: apr_file_info.c 
	gcc -o apr_file_info ${APR_FLAGS} ${APU_FLAGS} apr_file_info.c
clean:
	rm -f apr_file_info *.o 
