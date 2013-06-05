CURL_FLAGS = `pkg-config --cflags --libs glib-2.0` 
all: ghashtable
ghashtable: ghashtable.c
	gcc -std=c99 -o ghashtable ${CURL_FLAGS} ghashtable.c
clean:
	rm -f ghashtable *.o 
