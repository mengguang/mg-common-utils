CURL_FLAGS = `pkg-config --cflags --libs libcurl` 
all: curl_easy
curl_easy: curl_easy.c 
	gcc -std=c99 -o curl_easy ${CURL_FLAGS} curl_easy.c
clean:
	rm -f curl_easy *.o 
