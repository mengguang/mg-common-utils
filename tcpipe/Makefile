all: tcpipe_server tcpipe_client
tcpipe_server: server.cpp anet.o lib.o
	g++ -g -Wall -o tcpipe_server server.cpp anet.o lib.o -I . -lpthread
tcpipe_client: client.cpp anet.o lib.o
	g++ -g -Wall -o tcpipe_client client.cpp anet.o lib.o -I . -lpthread
anet.o: anet.h anet.cpp
	g++ -g -Wall -c anet.cpp -I .
lib.o: lib.h lib.cpp
	g++ -g -Wall -c lib.cpp -I .
clean:
	rm -f *.o tcpipe_server tcpipe_client
		
