all: BinlogProcessor.so 

BinlogProcessor.so: BinlogProcessor.cpp BinlogTypes.h BinlogRow.h BinlogTable.h 
	g++ -fPIC -Wall -shared -o BinlogProcessor.so BinlogProcessor.cpp -I/usr/local/weibo/py264/include/python2.6/

clean:
	rm -f BinlogProcessor.o BinlogProcessor.so 
