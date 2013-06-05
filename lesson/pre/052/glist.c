#include <stdio.h>
#include <glib.h>
#include <string.h>

void g_func_print(gpointer data,gpointer user_data) {
    printf("%s\n",data);
}

int main(int argc,char **argv) {

    GSList *sl=NULL;
    sl=g_slist_append(sl,"laomeng188@163.com");
    sl=g_slist_append(sl,"laozhang@gmail.com");
    sl=g_slist_prepend(sl,"laoli@qq.com");
    printf("length of list: %d\n",g_slist_length(sl));

    g_slist_foreach(sl,g_func_print,NULL);

    for(GSList *s=sl;s!=NULL;s=g_slist_next(s)) {
        printf("for loop: %s\n",s->data);
    }
    printf("\n");

    sl=g_slist_insert(sl,"xiaowang@163.com",1); 
    g_slist_foreach(sl,g_func_print,NULL);
    printf("\n");

    sl=g_slist_insert(sl,"xiaoxia@163.com",1); 
    g_slist_foreach(sl,g_func_print,NULL);
    printf("\n");

    sl=g_slist_delete_link(sl,g_slist_nth(sl,1));
    g_slist_foreach(sl,g_func_print,NULL);
    printf("\n");

    sl=g_slist_delete_link(sl,g_slist_nth(sl,1));
    g_slist_foreach(sl,g_func_print,NULL);
    printf("\n");
   
    g_slist_free(sl);

    GList *l=NULL;
    l=g_list_append(l,"a");
    l=g_list_prepend(l,"b");
    l=g_list_insert(l,"c",1);
    printf("first element: %s\n",g_list_first(l)->data);
    printf("last element: %s\n",g_list_last(l)->data);
    printf("previous element: %s\n",g_list_previous(g_list_last(l))->data);
    
    
    return 0;
}

