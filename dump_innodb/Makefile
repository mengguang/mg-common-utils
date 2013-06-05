all:dump_innodb

dump_innodb: dump_innodb.c test0aux.c test0aux.h
	gcc -g -o dump_innodb dump_innodb.c test0aux.c -I. -I/opt/haildb/include /opt/haildb23/lib/libhaildb.a -lz -lpthread
clean:
	rm -rf dump_innodb
