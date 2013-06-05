CURL_FLAGS = `pkg-config --cflags --libs libcurl` 
all: curl_post
curl_post: curl_post.c 
	gcc -std=c99 -o curl_post ${CURL_FLAGS} curl_post.c
clean:
	rm -f curl_post *.o 
