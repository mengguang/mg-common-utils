#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>

void my_free(gpointer data) {
    GString *s=(GString *)data;
    printf("freeing: %s\n",s->str);
    g_string_free(s,TRUE);
    return;
}

void g_hash_table_print_gstring(gpointer key,gpointer val,gpointer user_data) {
    GString *skey=(GString *)key;
    GString *sval=(GString *)val;
    printf("%s => %s\n",skey->str,sval->str);
    return;
}

guint my_hash(gconstpointer key){
    return g_string_hash((GString *)key);
}

gboolean my_equal(gconstpointer a, gconstpointer b) {
    return g_string_equal((GString *)a,(GString *)b);
}

int main(int argc,char **argv) {

    GHashTable *ht=g_hash_table_new(g_str_hash,g_str_equal);
    g_hash_table_destroy(ht);
    
    ht=g_hash_table_new_full(my_hash,my_equal,my_free,my_free);
    
    GString *key,*val;
    key=g_string_new("laomeng");
    val=g_string_new("laomeng188@163.com");
    g_hash_table_insert(ht,key,val);
    
    key=g_string_new("laozhang");
    val=g_string_new("laozhang@163.com");
    g_hash_table_insert(ht,key,val);

    key=g_string_new("laoli");
    val=g_string_new("laoli@qq.com");
    g_hash_table_insert(ht,key,val);

    g_hash_table_foreach(ht,g_hash_table_print_gstring,NULL);

    g_hash_table_remove_all(ht);
    g_hash_table_destroy(ht);

    return 0;
}

