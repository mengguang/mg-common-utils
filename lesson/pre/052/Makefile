CURL_FLAGS = `pkg-config --cflags --libs glib-2.0` 
all: glist
glist: glist.c
	gcc -std=c99 -o glist ${CURL_FLAGS} glist.c
clean:
	rm -f glist *.o 
