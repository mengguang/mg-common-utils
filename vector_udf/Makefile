all:libvector.so

libvector.so:vector.c
	gcc -std=c99 -g -fPIC -shared -o libvector.so vector.c -I /usr/include/mysql/  -lrt

install:
	mysql test -e "drop function if exists zadd"
	mysql test -e "drop function if exists zrange"
	mysql test -e "drop function if exists zcard"
	mysql test -e "drop function if exists zrem"
	cp libvector.so /usr/lib64/mysql/plugin/
	mysql test -e "create function zadd returns string soname 'libvector.so'"
	mysql test -e "create function zrange returns string soname 'libvector.so'"
	mysql test -e "create function zcard returns int soname 'libvector.so'"
	mysql test -e "create function zrem returns string soname 'libvector.so'"

uninstall:
	mysql test -e "drop function if exists zadd"
	mysql test -e "drop function if exists zrange"
	mysql test -e "drop function if exists zcard"
	mysql test -e "drop function if exists zrem"
clean:
	rm -f *.o *.so
