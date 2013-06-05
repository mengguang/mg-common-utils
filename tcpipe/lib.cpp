#include <lib.h>

bool simpleEncode(char *str,int len) {
    for(int i=0;i<len;i++) {
        str[i]+=3;
    }
    return true;
}

bool simpleDecode(char *str,int len) {
    for(int i=0;i<len;i++) {
        str[i]-=3;
    }
    return true;
}
