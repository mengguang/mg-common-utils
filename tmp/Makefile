LIBS=hiredis/net.o hiredis/hiredis.o hiredis/sds.o hiredis/async.o fcgi/libfcgi/.libs/libfcgi.a
INCLUDE= -I ./hiredis -I./fcgi/include/ 
OPT= -std=c99 -g -O2 -Wall

all: aj_count_new i_set_new
aj_count_new: aj_count_new.c
	gcc ${OPT} -o aj_count_new tinyini.c aj_count_new.c $(LIBS) $(INCLUDE)

i_set_new: i_set_new.c
	gcc ${OPT} -o i_set_new tinyini.c i_set_new.c $(LIBS) $(INCLUDE)
clean:
	rm -rf *.o aj_count_new i_set_new

run: aj_count_new i_set_new
	pkill aj_count_new || true
	pkill i_set_new || true
	./spawn-fcgi/src/spawn-fcgi -f ./aj_count_new -s /tmp/aj_count_new.sock -F 1
	chmod 666 /tmp/aj_count_new.sock
	./spawn-fcgi/src/spawn-fcgi -f ./i_set_new -s /tmp/i_set_new.sock -F 1
	chmod 666 /tmp/i_set_new.sock
