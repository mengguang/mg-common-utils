# Redis Makefile
# Copyright (C) 2009 Salvatore Sanfilippo <antirez at gmail dot com>
# This file is released under the BSD license, see the COPYING file

OPTIMIZATION?=-O3
CFLAGS?= -std=c99 -finline-functions -rdynamic -pedantic $(OPTIMIZATION) -Wall $(ARCH) $(PROF)
CCLINK?= -pthread -ldl
CCOPT= $(CFLAGS) $(CCLINK) $(ARCH) $(PROF)
#DEBUG?= -g -rdynamic -ggdb 

INSTALL_TOP= /usr/local
INSTALL_BIN= $(INSTALL_TOP)/bin
INSTALL= cp -p

OBJ = adlist.o ae.o anet.o redis.o

PRGNAME = redis-server

all: redis-server echo.so

# Deps (use make dep to generate this)
adlist.o: adlist.c adlist.h
ae.o: ae.c ae.h
ae_epoll.o: ae_epoll.c
anet.o: anet.c anet.h
redis.o: redis.c redis.h ae.h anet.h adlist.h
echo.so: echo.c redis.h

redis-server: $(OBJ)
	$(CC) -o $(PRGNAME) $(CCOPT) $(DEBUG) $(OBJ)

echo.so:
	gcc -shared -fPIC -o echo.so echo.c

.c.o:
	$(CC) -c $(CFLAGS) $(DEBUG) $(COMPILE_TIME) $<

clean:
	rm -rf $(PRGNAME) *.o *.gcda *.gcno *.gcov *.so

dep:
	$(CC) -MM *.c
	
install: all
	$(INSTALL) $(PRGNAME) $(INSTALL_BIN)