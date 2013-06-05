#include <stdio.h>
#include <string.h>

int main(int argc,char *argv) {
    
    char *name = "laomeng";
    printf("name = %s \n",name); 
    char *email = "laomeng188@163.com";
    char *blog = "http://laomeng188.blog.163.com";
    
    printf("length of name: %d\n",strlen(name));
    printf("length of email: %d\n",strlen(email));

    printf("sizeof name: %d\n",sizeof(name));
    printf("sizeof email: %d\n",sizeof(email));

    for(int i=0; i< strlen(name); i++) {
        printf("%d => %c\n",i,name[i]);
    }

    //char name[128];
    //name[2]='x';
   
    return 0;
}

