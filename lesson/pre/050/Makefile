CURL_FLAGS = `pkg-config --cflags --libs glib-2.0` 
all: gstring
gstring: gstring.c
	gcc -std=c99 -o gstring ${CURL_FLAGS} gstring.c
clean:
	rm -f gstring *.o 
