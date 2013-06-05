all: sock4a
sock4a: sock4a.c
	gcc -o sock4a -DLINUX=2 -D_REENTRANT -D_GNU_SOURCE -pthread -I/usr/include/apr-1  -lapr-1 -lpthread sock4a.c
clean:
	rm -f *.o sock4a
