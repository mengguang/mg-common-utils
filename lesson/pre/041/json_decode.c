#include <stdio.h>
#include <string.h>
#include <json.h>
void json_print_value(json_object *obj);
void json_print_array(json_object *obj);
void json_print_object(json_object *obj);

void json_print_value(json_object *obj) {
    if(!obj) return;
    json_type type=json_object_get_type(obj);
    if(type == json_type_boolean) {
        if(json_object_get_boolean(obj)) {
            printf("true");
        } else {
            printf("false");
        }
    } else if(type == json_type_double) {
        printf("%lf",json_object_get_double(obj));
    } else if(type == json_type_int) {
        printf("%d",json_object_get_int(obj));
    } else if(type == json_type_string) {
        printf("%s",json_object_get_string(obj));
    } else if(type == json_type_object) {
        json_print_object(obj);
    } else if(type == json_type_array) {
        json_print_array(obj);
    } else {
        printf("ERROR");
    }
    printf(" ");
}

void json_print_array(json_object *obj) {
    if(!obj) return;
    int length=json_object_array_length(obj);
    for(int i=0;i<length;i++) {
        json_object *val=json_object_array_get_idx(obj,i);
        json_print_value(val);
    }
}

void json_print_object(json_object *obj) {
    if(!obj) return;
    json_object_object_foreach(obj,key,val) {
        printf("%s => ",key);
        json_print_value(val);
    }
}

int main(int argc,char **argv) {

    char buf[1024];
    while(!feof(stdin)) {
        memset(buf,0,1024);
        fgets(buf,1023,stdin);
        //fputs(buf,stdout);
        json_object *obj=json_tokener_parse(buf);
        json_print_value(obj);
        json_object_put(obj);
        printf("\n");
    }

    return 0;
}

