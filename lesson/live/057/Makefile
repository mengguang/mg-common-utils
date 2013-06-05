CURL_FLAGS = `pkg-config --cflags --libs glib-2.0` 
all: gkeyfile
gkeyfile: gkeyfile.c
	gcc -std=c99 -o gkeyfile ${CURL_FLAGS} gkeyfile.c
clean:
	rm -f gkeyfile *.o 
