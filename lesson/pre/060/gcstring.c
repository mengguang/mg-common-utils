#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>


int main(int argc,char **argv) {

    gchar *s=g_strdup("laomeng");
    printf("s: %s\n",s);
    g_free(s);
    s=g_strndup("laomeng",3);
    printf("s: %s\n",s);
    g_free(s);
    s=g_strdup("laozhang,laoyang,laoli,laomeng");
    gchar **ss=g_strsplit(s,",",0);
    g_free(s);
    
    int i=0;
    while(ss[i] != NULL) {
        printf("%s\n",ss[i]);
        i++;
    }
    s=g_strjoinv("|",ss);
    printf("join result: %s\n",s);
    s=g_strreverse(s);
    printf("reverse result: %s\n",s);
    g_free(s);


    g_strfreev(ss);


    return 0;
}

