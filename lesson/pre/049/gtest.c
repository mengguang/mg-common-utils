#include <glib.h>

int main(int argc,char **argv) {

    gint x=100; 
    guint64 u=100000000000000L;
    g_print("x = %d, u=%llu\n",x,u);
    return 0;
}

