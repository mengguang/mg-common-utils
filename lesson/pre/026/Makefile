APR_FLAGS = -DLINUX=2 -D_REENTRANT -D_GNU_SOURCE -pthread -I/usr/include/apr-1  -lapr-1 -lpthread
APU_FLAGS = -DLINUX=2 -D_REENTRANT -D_GNU_SOURCE -pthread -I/usr/include/apr-1  -laprutil-1 -lldap -llber -lexpat -ldb-4.7 -lapr-1 -lpthread
all: apr_dbd_mysql
apr_dbd_mysql: apr_dbd_mysql.c 
	gcc -std=c99 -o apr_dbd_mysql ${APR_FLAGS} ${APU_FLAGS} apr_dbd_mysql.c
clean:
	rm -f apr_dbd_mysql *.o 
