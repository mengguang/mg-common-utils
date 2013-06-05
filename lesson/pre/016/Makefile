APR_FLAGS = `pkg-config --cflags --libs apr-1` 
all: thread
thread: thread.c 
	gcc -o thread ${APR_FLAGS} -lpthread thread.c
clean:
	rm -f thread *.o
