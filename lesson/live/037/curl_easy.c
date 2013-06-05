#include <stdio.h>
#include <curl/curl.h>

int main(int argc,char **argv) {

    char *url;
    if(argc >1 ) {
        url=argv[1];
    }else {
        url="http://apr.apache.org/docs/apr/1.3/modules.html";
    }

    CURL *curl;
    CURLcode result;
    curl=curl_easy_init();
    if(!curl) {
        return -1;
    }
    FILE *fp=fopen("result.htm","w+");
    if(!fp) {
        perror("fopen");
        return -2;
    }
    curl_easy_setopt(curl,CURLOPT_URL,url);
    curl_easy_setopt(curl,CURLOPT_VERBOSE,1L);
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,fp);

    result=curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);

    return 0;
}

