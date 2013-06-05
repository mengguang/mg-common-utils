GLIB_FLAGS = `pkg-config --cflags --libs glib-2.0` 
all: gtest
gtest: gtest.c
	gcc -std=c99 -o gtest ${GLIB_FLAGS} gtest.c
clean:
	rm -f gtest *.o 
