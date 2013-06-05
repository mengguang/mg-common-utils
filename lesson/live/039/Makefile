CURL_FLAGS = `pkg-config --cflags --libs libcurl` 
all: curl_ftp_get
curl_ftp_get: curl_ftp_get.c 
	gcc -std=c99 -o curl_ftp_get ${CURL_FLAGS} curl_ftp_get.c
clean:
	rm -f curl_ftp_get *.o 
