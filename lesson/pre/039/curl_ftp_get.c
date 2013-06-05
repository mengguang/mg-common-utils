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
    char *url;
    if(argc > 1) {
        url=argv[1];
    } else {
        url="ftp://ftp001:123qwe@localhost/test.txt";
    }
    curl_easy_setopt(curl,CURLOPT_URL,url);

    FILE *fp=fopen("result.txt","w+");
    if(!fp) {
        perror("fopen");
        return -2;
    }
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,fp);
    curl_easy_setopt(curl,CURLOPT_VERBOSE,1L);
    
    result=curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);

    return 0;
}

