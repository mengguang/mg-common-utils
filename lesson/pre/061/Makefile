CURL_FLAGS = `pkg-config --cflags --libs glib-2.0` 
all: gdir
gdir: gdir.c
	gcc -std=c99 -o gdir ${CURL_FLAGS} gdir.c
clean:
	rm -f gdir *.o 
