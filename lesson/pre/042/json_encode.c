#include <stdio.h>
#include <string.h>
#include <json.h>

int main(int argc,char **argv) {

    json_object *json=json_object_new_object();
    json_object_object_add(json,"name",json_object_new_string("laomeng"));
    json_object_object_add(json,"email",json_object_new_string("laomeng188@163.com"));
    json_object_object_add(json,"age",json_object_new_int(30));
    json_object *tech=json_object_new_array();
    json_object_array_add(tech,json_object_new_string("c"));
    json_object_array_add(tech,json_object_new_string("c++"));
    json_object_array_add(tech,json_object_new_string("php"));
    json_object_object_add(json,"technology",tech);
    const char *str=json_object_to_json_string(json);
    printf("%s\n",str);
    json_object_put(json);
    return 0;
}

