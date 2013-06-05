#include <stdio.h>
#include <string.h>
#include <mysql.h>
#include <stdlib.h>

my_bool zadd_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void zadd_deinit(UDF_INIT *initid);
char *zadd(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned *res_length, char *null_value, char *error);

my_bool zrem_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void zrem_deinit(UDF_INIT *initid);
char *zrem(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned *res_length, char *null_value, char *error);

my_bool zcard_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void zcard_deinit(UDF_INIT *initid);
long long zcard(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error);

my_bool zrange_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
void zrange_deinit(UDF_INIT *initid);
char *zrange(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned *res_length, char *null_value, char *error);

struct object {
    unsigned int member;
    unsigned int score;
};
typedef struct object object;

#define MAX_VECTOR_LEN 16384

static int object_cmp(const void *p1, const void *p2)
{
    return ((object *)p1)->score - ((object *)p2)->score;
}

/*
 * select zadd(old_data,new_member,new_score)
 */

my_bool zadd_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    if(args->arg_count != 3) {
        strcpy(message,"Usage: select zadd(old_data,new_member,new_score).");
        return 1;
    }
    if(args->arg_type[0] != STRING_RESULT || args->arg_type[1] != INT_RESULT || args->arg_type[2] != INT_RESULT) {
        strcpy(message,"Usage: select zadd(string,int,int).");
        return 1;
    }
    if( *(long long *)args->args[1] < 0 || *(long long *)args->args[1] > 4294967295
        || *(long long *)args->args[2] < 0 || *(long long *)args->args[2] > 4294967295) {
            strcpy(message,"Usage: member and score must be a number bewteen 0 and 4294967295.");
            return 1;
    }
    void *mptr=malloc(MAX_VECTOR_LEN);
    if(!mptr) {
        strcpy(message,"Memory allocation failed.");
        return 1;
    }
    initid->max_length=MAX_VECTOR_LEN;
    initid->ptr=mptr;
    initid->const_item=0;
    return 0;
}

void zadd_deinit(UDF_INIT *initid)
{
    if(initid->ptr != NULL) {
        free(initid->ptr);
    }
    return;
}

char *zadd(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned *res_length, char *null_value, char *error)
{
    object *old_data=(object *)args->args[0];
    int len=args->lengths[0];
    memset(initid->ptr,0,initid->max_length);
    if(len > initid->max_length) {
        memcpy(initid->ptr,old_data,initid->max_length);
        *res_length=initid->max_length;
        return initid->ptr;
    }
    if( len % sizeof(object) != 0 ) {
        memcpy(initid->ptr,old_data,len);
        *res_length=len;
        return initid->ptr;
    }
    object obj;
    obj.member=*(long long *)args->args[1];
    obj.score=*(long long *)args->args[2];
    int found=0;
    memcpy(initid->ptr,old_data,len);
    object *new_data=(object *)initid->ptr;
    for(int i=0;i<len/sizeof(object);i++){
        if( new_data[i].member == obj.member ) {
            new_data[i].score = obj.score;
            found = 1;
        }
    }
    if(found == 0 && len + sizeof(object) <= initid->max_length) {   
        memcpy(initid->ptr+len,&obj,sizeof(object));
        *res_length=len+sizeof(object);
    }
    else {
        *res_length=len;
    }
    qsort(initid->ptr,*res_length / sizeof(object),sizeof(object),object_cmp);
    return initid->ptr;
}

my_bool zrange_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    if(args->arg_count != 3) {
        strcpy(message,"Usage: select zrange(data,start,stop).");
        return 1;
    }
    if(args->arg_type[0] != STRING_RESULT || args->arg_type[1] != INT_RESULT || args->arg_type[2] != INT_RESULT) {
        strcpy(message,"Usage: select zrange(string,int,int).");
        return 1;
    }
    if( *(long long *)args->args[1] < 0 || *(long long *)args->args[1] > 2000
        || *(long long *)args->args[2] < 0 || *(long long *)args->args[2] > 2000) {
            strcpy(message,"Usage: member and score must a number between 0 and 2000");
            return 1;
    }
    void *mptr=malloc(MAX_VECTOR_LEN*3);
    if(!mptr) {
        strcpy(message,"Memory allocation failed.");
        return 1;
    }
    initid->max_length=MAX_VECTOR_LEN*3;
    initid->ptr=mptr;
    initid->const_item=0;
    return 0;
}

void zrange_deinit(UDF_INIT *initid)
{
    if(initid->ptr != NULL) {
        free(initid->ptr);
    }
    return;
}

char *zrange(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned *res_length, char *null_value, char *error)
{
    object *objs=(object *)args->args[0];
    int len=args->lengths[0];
    char *pos=initid->ptr;
    if(len > MAX_VECTOR_LEN ) {
        *null_value=1;
        return NULL;
    }
    memset(initid->ptr,0,initid->max_length);
    if( len % sizeof(object) != 0) {
        *null_value=1;
        return NULL;
    }
    unsigned int nobj=len / sizeof(object);
    unsigned int start=*(long long *)args->args[1];
    unsigned int stop=*(long long *)args->args[2];
    if(stop > nobj) {
        stop=nobj;
    }
    *pos='[';
    ++pos;
    for(int i=start;i < stop ;i++) {
        object obj=objs[i];
        pos+=sprintf(pos,"[%u,%u]",obj.member,obj.score);
        if(stop - i != 1) {
            *pos=',';
            ++pos;
        }
    }
    *pos=']';
    ++pos;
    *res_length=pos - (char *)initid->ptr;
    return initid->ptr;
}


my_bool zcard_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    if(args->arg_count != 1) {
        strcpy(message,"Usage: select zcard(data).");
        return 1;
    }
    if(args->arg_type[0] != STRING_RESULT ) {
        strcpy(message,"Usage: select zcard(string).");
        return 1;
    }
    return 0;
}

void zcard_deinit(UDF_INIT *initid)
{
    return;
}

long long zcard(UDF_INIT *initid, UDF_ARGS *args, char *is_null, char *error)
{
    int len=args->lengths[0];
    if(len > MAX_VECTOR_LEN ) {
        return -1;
    }
    if( len % sizeof(object) != 0) {
        return -1;
    }
    long long nobj=len / sizeof(object);
    return nobj;
}


my_bool zrem_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
    if(args->arg_count != 2) {
        strcpy(message,"Usage: select zrem(data,member).");
        return 1;
    }
    if(args->arg_type[0] != STRING_RESULT || args->arg_type[1] != INT_RESULT ) {
        strcpy(message,"Usage: select zadd(string,int,int).");
        return 1;
    }
    if( *(long long *)args->args[1] < 0 || *(long long *)args->args[1] > 4294967295) {
            strcpy(message,"Usage: member must be a number between 0 and 4294967295." );
            return 1;
    }
    void *mptr=malloc(MAX_VECTOR_LEN);
    if(!mptr) {
        strcpy(message,"Memory allocation failed.");
        return 1;
    }
    initid->max_length=MAX_VECTOR_LEN;
    initid->ptr=mptr;
    initid->const_item=0;
    return 0;
}

void zrem_deinit(UDF_INIT *initid)
{
    if(initid->ptr != NULL) {
        free(initid->ptr);
    }
    return;
}

char *zrem(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned *res_length, char *null_value, char *error)
{
    memset(initid->ptr,0,initid->max_length);
    object *old_data=(object *)args->args[0];
    int len=args->lengths[0];
    if(len + sizeof(object) > initid->max_length || len % sizeof(object) != 0 ) {
        memcpy(initid->ptr,old_data,len > initid->max_length ? initid->max_length : len);
        *res_length=len > initid->max_length ? initid->max_length : len;
        return initid->ptr;
    }
    unsigned int member=*(long long *)args->args[1];
    int k=0;
    for(int i=0;i<len/sizeof(object);i++) {
        if( old_data[i].member != member ) {
            memcpy(initid->ptr+k*sizeof(object),&old_data[i],sizeof(object));
            k++;
        }
    }
    *res_length=k*sizeof(object);
    return initid->ptr;
}
