#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>

struct Person {
    int id;
    char name[128];
    char email[128];
    int age;
};

int main(int argc,char **argv) {

    char *s=g_slice_alloc(64);
    for(int i=0;i<64;i++) {
        printf("%d ",s[i]);
    }
    printf("\n");
    g_slice_free1(64,s);
    
    s=g_slice_alloc0(64);
    for(int i=0;i<64;i++) {
        printf("%d ",s[i]);
    }
    printf("\n");
    g_slice_free1(64,s);

    struct Person *p1=g_slice_new(struct Person);
    struct Person *p2=g_slice_new0(struct Person);
    struct Person *p3=g_slice_dup(struct Person,p2);

    g_slice_free(struct Person,p1);
    g_slice_free(struct Person,p2);
    g_slice_free(struct Person,p3);
    

    return 0;
}


