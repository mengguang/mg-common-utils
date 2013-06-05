#include <stdio.h>
#include <curl/curl.h>
#include <string.h>

int main(int argc,char **argv) {

    char *url="http://localhost/v/post.php?email=laomeng188@163.com";
    CURL *curl;
    CURLcode result;
    curl=curl_easy_init();
    if(!curl) {
        return -1;
    }
    curl_easy_setopt(curl,CURLOPT_URL,url);
    curl_easy_setopt(curl,CURLOPT_POST,1L);
    char *post="name=laomeng188&age=30";
    curl_easy_setopt(curl,CURLOPT_POSTFIELDS,post);
    FILE *fp=fopen("result.htm","w+");
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

