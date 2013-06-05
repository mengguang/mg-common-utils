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
        url="http://10.1.9.140/v/post.php";
    }
    curl_easy_setopt(curl,CURLOPT_URL,url);
    curl_easy_setopt(curl,CURLOPT_POST,1);
    char *postfields="name=laomeng188@163.com&age=30";
    curl_easy_setopt(curl,CURLOPT_POSTFIELDS,postfields);
    curl_easy_setopt(curl,CURLOPT_POSTFIELDSIZE,strlen(postfields));

    FILE *fp=fopen("result.htm","w+");
    if(!fp) {
        perror("fopen");
        return -2;
    }
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,fp);
    curl_easy_setopt(curl,CURLOPT_VERBOSE,1);
    
    result=curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(fp);

    return 0;
}

