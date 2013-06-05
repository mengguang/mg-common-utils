#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>


int main(int argc,char **argv) {

    gchar *s=g_strdup("laozhang");
    GQuark x=g_quark_from_string(s);
    printf("quark of %s is %u\n",s,x);
    g_free(s);
    s=g_strdup("laomeng");
    GQuark y=g_quark_from_string(s);
    printf("quark of %s is %u\n",s,y);
    const gchar *ss=g_quark_to_string(y);
    printf("%u : %s\n",y,ss);
    const gchar *vv=g_intern_string("laomeng");
    const gchar *uu=g_intern_string(s); 
    printf("vv : %p\n",vv);
    printf("uu : %p\n",uu);
    g_free(s);



    return 0;
}

