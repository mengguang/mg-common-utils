all:repl4
repl4:repl4.c
	gcc -o repl4 repl4.c  `mysql_config --libs` -I /usr/include/mysql -I /root/src/Percona-Server/include/ -I /root/src/Percona-Server/sql -I /root/src/Percona-Server/regex

clean:
	rm -f *.o repl4
