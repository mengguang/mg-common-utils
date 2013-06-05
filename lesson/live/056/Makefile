CURL_FLAGS = `pkg-config --cflags --libs glib-2.0` 
all: gchecksum2
gchecksum2: gchecksum2.c
	gcc -std=c99 -o gchecksum2 ${CURL_FLAGS} gchecksum2.c
clean:
	rm -f gchecksum2 *.o 
