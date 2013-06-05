CURL_FLAGS = `pkg-config --cflags --libs glib-2.0` 
all: gtimer
gtimer: gtimer.c
	gcc -std=c99 -o gtimer ${CURL_FLAGS} gtimer.c
clean:
	rm -f gtimer *.o 
