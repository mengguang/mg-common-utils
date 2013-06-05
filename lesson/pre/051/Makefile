CURL_FLAGS = `pkg-config --cflags --libs glib-2.0` 
all: garray
garray: garray.c
	gcc -std=c99 -o garray ${CURL_FLAGS} garray.c
clean:
	rm -f garray *.o 
