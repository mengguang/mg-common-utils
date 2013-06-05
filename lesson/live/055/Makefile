CURL_FLAGS = `pkg-config --cflags --libs glib-2.0` 
all: gchecksum
gchecksum: gchecksum.c
	gcc -std=c99 -o gchecksum ${CURL_FLAGS} gchecksum.c
clean:
	rm -f gchecksum *.o 
