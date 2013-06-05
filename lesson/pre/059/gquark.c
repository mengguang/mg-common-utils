#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>


int main(int argc,char **argv) {

    gchar *s=g_strdup("laozhang");
    GQuark x=g_quark_from_string(s);
    printf("quark of %s is %u\n",s,x);
    printf("string of quark %u is %s\n",x,g_quark_to_string(x));
    const gchar *ss=g_intern_string(s);
    printf("ss: %s\n",ss);
    printf("address of ss: \t%p\n",ss);
    printf("address quark: \t%p\n",g_quark_to_string(x));
    g_free(s);
    s=g_strdup("laoli");
    x=g_quark_from_string(s);
    printf("quark of %s is %u\n",s,x);
    printf("string of quark %u is %s\n",x,g_quark_to_string(x));


    return 0;
}

