CURL_FLAGS = `pkg-config --cflags --libs glib-2.0` 
all: gslice
gslice: gslice.c
	gcc -std=c99 -o gslice ${CURL_FLAGS} gslice.c
clean:
	rm -f gslice *.o 
