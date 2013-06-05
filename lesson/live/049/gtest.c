#include <glib.h>

int main(int argc,char **argv) {

    gint x=100;
    guint64 y=1000000000000000L;
    g_print("x = %d, y = %llu\n",x,y); 
    return 0;
}


