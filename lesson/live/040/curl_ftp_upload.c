#include <stdio.h>
#include <curl/curl.h>
#include <string.h>

int main(int argc,char **argv) {
    CURL *curl;
    CURLcode result;
    curl=curl_easy_init();
    if(!curl) {
        return -1;
    }

    char *url="ftp://ftp001:123qwe@localhost/upload.txt";
    curl_easy_setopt(curl,CURLOPT_URL,url);
    FILE *fp=fopen("upload.txt","r");
    if(!fp) {
        perror("fopen");
        return -2;
    }
    curl_easy_setopt(curl,CURLOPT_UPLOAD,1L);
    curl_easy_setopt(curl,CURLOPT_READDATA,fp);
    curl_easy_setopt(curl,CURLOPT_VERBOSE,1);

    result=curl_easy_perform(curl);

    fclose(fp);

    curl_easy_cleanup(curl);

    return 0;
}

