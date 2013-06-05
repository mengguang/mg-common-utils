all: server client
server: server.cpp anet.o lib.o
	g++ -g -Wall -o server server.cpp anet.o lib.o -I . -lpthread
client: client.cpp anet.o lib.o
	g++ -g -Wall -o client client.cpp anet.o lib.o -I . -lpthread
anet.o: anet.h anet.cpp
	g++ -g -Wall -c anet.cpp -I .
lib.o: lib.h lib.cpp
	g++ -g -Wall -c lib.cpp -I .
clean:
	rm -f *.o server client
		
