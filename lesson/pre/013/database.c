#include <stdio.h>
#include <string.h>

struct Person {
    int id;
    int age;
    char name[32];
    char email[32];
};

void printPerson(struct Person p) {
    printf("id = %d, age = %d, name = %s, email = %s\n",
        p.id,p.age,p.name,p.email);
    return;
}

int main(int argc,char **argv) {

    char *file;
    if(argc > 1) {
        file=argv[1];
    } else {
        file="person.db";
    }

    FILE *wfp=fopen(file,"a");
    if(!wfp) {
        perror("wfp fopen:");
        return -1;
    }

    struct Person p;
    p.id=2;
    p.age=28;
    strcpy(p.name,"laowang");
    strcpy(p.email,"laowang188@163.com");
    fwrite(&p,sizeof(struct Person),1,wfp);
    fclose(wfp);

    FILE *rfp=fopen(file,"r");
    if(!rfp) {
        perror("rfp fopen:");
        return -2;
    }
    
    struct Person person;
    int nread=0;
    while(!feof(rfp)) {
        nread=fread(&person,sizeof(person),1,rfp);
        if(nread == 1) {
            printPerson(person);
        }
    }
    fclose(rfp);
    return 0;
}
