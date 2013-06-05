#include <stdio.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>

void loop_func() {
    long k=1;
    for(int i=0;i<1000000;i++) {
        k+=i;
    }
}

int main(int argc,char **argv) {

    int times=100;
    if(argc > 1) {
        times=atoi(argv[1]);
    }
    GTimer *tm=g_timer_new();
    g_timer_start(tm);
    for(;times > 0;times--) {
        loop_func();
    }

    g_timer_stop(tm);
    gdouble elapsed=g_timer_elapsed(tm,NULL);
    printf("time used: %lf\n",elapsed);
    g_timer_destroy(tm);
    
    return 0;
}

