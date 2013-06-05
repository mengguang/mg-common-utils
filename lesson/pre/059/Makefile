CURL_FLAGS = `pkg-config --cflags --libs glib-2.0` 
all: gquark
gquark: gquark.c
	gcc -std=c99 -o gquark ${CURL_FLAGS} gquark.c
clean:
	rm -f gquark *.o 
