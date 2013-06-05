CURL_FLAGS = `pkg-config --cflags --libs libcurl` 
all: curl_ftp_upload
curl_ftp_upload: curl_ftp_upload.c 
	gcc -std=c99 -o curl_ftp_upload ${CURL_FLAGS} curl_ftp_upload.c
clean:
	rm -f curl_ftp_upload *.o 
