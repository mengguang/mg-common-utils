#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>

int main(int argc,char **argv) {

    char *filename="user.ini";
    GKeyFile *kf=g_key_file_new();
    GError *err=NULL;
    if(!g_key_file_load_from_file(kf,filename,G_KEY_FILE_NONE,&err) ){
        fprintf(stderr,"error: %s\n",err->message);
        g_error_free(err);
        return -1;
    }
    gsize len=0;
    gchar **groups=g_key_file_get_groups(kf,&len);

    for(gsize i=0;i<len;i++) {
        printf("group: %s\n",groups[i]);
        gsize nkey=0;
        gchar **keys=g_key_file_get_keys(kf,groups[i],&nkey,&err);
        if(err != NULL) {
            fprintf(stderr,"error: %s\n",err->message);
            g_error_free(err);
            return -1;
        }
        for(gsize ki=0;ki<nkey;ki++) {
            printf("key : %s\n",keys[ki]);
            gchar *val=g_key_file_get_string(kf,groups[i],keys[ki],&err);
            if(err != NULL) {
                fprintf(stderr,"error: %s\n",err->message);
                g_error_free(err);
                return -1;
            }
            printf("value of key %s: %s\n",keys[ki],val);
            g_free(val);


        }
        g_strfreev(keys);

    }

    g_strfreev(groups);
    return 0;
}

