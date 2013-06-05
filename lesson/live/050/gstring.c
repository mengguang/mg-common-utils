#include <glib.h>
#include <stdio.h>

int main(int argc,char **argv) {


    GString *s=g_string_new("Hello World.");
    g_print("From GString: %s\n",s->str);
    printf("Length: %d\n",s->len);
    printf("Capaticy: %d\n",s->allocated_len);


    GString *t=g_string_new("Hello Laomeng");
    if(g_string_equal(s,t)) {
        printf("equal\n");
    } else {
        printf("not equal\n");
    }

    g_string_ascii_up(s);
    g_print("From GString: %s\n",s->str);
    g_string_ascii_down(s);
    g_print("From GString: %s\n",s->str);

    g_string_append(s,", Hello Universe.");
    g_print("From GString: %s\n",s->str);
    printf("Length: %d\n",s->len);
    printf("Capaticy: %d\n",s->allocated_len);

    g_string_truncate(s,0);
    g_print("From GString: %s\n",s->str);
    printf("Length: %d\n",s->len);
    printf("Capaticy: %d\n",s->allocated_len);

    g_string_printf(s,"%d + %d = %d",100,200,100+200);
    g_print("From GString: %s\n",s->str);
    g_string_append_printf(s,"\t %d * %d = %d\n",100,200,100*200);
    g_print("From GString: %s\n",s->str);

    g_string_free(s,TRUE);


    return 0;
}

