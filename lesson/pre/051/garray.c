#include <stdio.h>
#include <glib.h>

gint cmp(gconstpointer a,gconstpointer b) {
    gint ai=*(gint *)a;
    gint bi=*(gint *)b;
    return ai-bi;
}

int main(int argc,char **argv) {

    GArray *a=g_array_new(TRUE,TRUE,sizeof(gint));
    printf("length of array: %d\n",a->len);
    for(gint i=0;i<10;i++) {
        g_array_append_val(a,i);
    }
    printf("length of array: %d\n",a->len);
    for(gint i=0;i<a->len;i++) {
        printf("%d ",g_array_index(a,gint,i));
    }
    printf("\n");
    for(gint i=0;i<10;i++) {
        g_array_prepend_val(a,i);
    }
    printf("length of array: %d\n",a->len);
    for(gint i=0;i<a->len;i++) {
        printf("%d ",g_array_index(a,gint,i));
    }
    printf("\n");

    gint k=10000;
    //g_array_insert_val(a,10,10000);
    g_array_insert_val(a,10,k);
    for(gint i=0;i<a->len;i++) {
        printf("%d ",g_array_index(a,gint,i));
    }
    printf("\n");
    g_array_remove_index(a,10);
    for(gint i=0;i<a->len;i++) {
        printf("%d ",g_array_index(a,gint,i));
    }
    printf("\n");

    g_array_sort(a,cmp);
    for(gint i=0;i<a->len;i++) {
        printf("%d ",g_array_index(a,gint,i));
    }
    printf("\n");
    
    g_array_free(a,TRUE);
    
    return 0;
}

