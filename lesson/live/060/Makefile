CURL_FLAGS = `pkg-config --cflags --libs glib-2.0` 
all: gcstring
gcstring: gcstring.c
	gcc -std=c99 -o gcstring ${CURL_FLAGS} gcstring.c
clean:
	rm -f gcstring *.o 
