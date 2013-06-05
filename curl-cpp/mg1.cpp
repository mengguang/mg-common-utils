#include <cstdio>
#include <curl/curl.h>
#include <iostream>

using namespace std;

int main(int argc,char **argv) {
    CURL *ch = curl_easy_init();
    if(ch == NULL) {
        cerr << "curl_easy_init error." << endl;
        return -1;
    }
    
    FILE *ofp = fopen("out.data","w+");
    if(ofp == NULL) {
        cerr << "fopen error." << endl;
        perror("fopen:");
        return -1;
    }
    curl_easy_setopt(ch,CURLOPT_WRITEDATA,ofp);
    curl_easy_setopt(ch,CURLOPT_URL,"http2://www.okbuy.comx");
    CURLcode code=curl_easy_perform(ch);
    if(code != 0) {
        cerr << "curl_easy_perform: " << curl_easy_strerror(code) << endl;
    }
    curl_easy_cleanup(ch);
    return 0;
}


