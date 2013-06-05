THRIFT_DIR = /opt/thrift/include/thrift
LIB_DIR = /opt/thrift/lib -L /opt/thrift/lib -Wl,-rpath=/opt/thrift/lib -lmysqlclient_r -L/usr/lib64/mysql

GEN_SRC = handler_constants.cpp  handler.cpp  HandlerService.cpp  handler_types.cpp


default: server client

server: server.cpp
	g++ -o server -I${THRIFT_DIR} -I. -L${LIB_DIR} -lthrift server.cpp ${GEN_SRC}

client: client.cpp
	g++ -o client -I${THRIFT_DIR} -I. -L${LIB_DIR} -lthrift client.cpp ${GEN_SRC}

clean:
	$(RM) -r client server
gen: handler.thrift
	/opt/thrift/bin/thrift -out . --gen cpp handler.thrift
