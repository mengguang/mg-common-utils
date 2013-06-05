import std.stdio, std.string, std.array, std.socket;
import std.concurrency;
import core.thread;

immutable BUF_SIZE=65536;
immutable LISTEN_PORT=21080;
immutable FAKE_CLOSE="mg_proxy_close_instruction_20111008";
immutable FAKE_CLOSE_LEN=35;
immutable char[] serverIP="";
immutable ushort port = 21080;

void main() {
    auto serverSocket=makeServer(port);
    while(true) {
        auto browserSocket=serverSocket.accept();
        writeln("got a connection!");
        StartThread(browserSocket);
    }
}

void StartThread(Socket sock) {
    struct Con {
        Socket sock;
        void Go() { WorkerThread( sock ); }
    }

    auto con = new Con;
    con.sock = sock; 
    auto worker = new Thread( &con.Go );
    worker.isDaemon(true);
    worker.start();
}

void WorkerThread(Socket browserSocket) {
    auto proxySocket=connectToServer(serverIP,port);

    auto writeSet=new SocketSet();
    auto readSet=new SocketSet();
    auto errorSet=new SocketSet();
    while(true) {
        readSet.reset();
        readSet.add(browserSocket);
        readSet.add(proxySocket);
        
        writeSet.reset();

        errorSet.reset();
        errorSet.add(browserSocket);
        errorSet.add(proxySocket);

        auto n=Socket.select(readSet,writeSet,errorSet);
        if( n == 0) {
            continue;
        }
        if( n == -1 ) {
            continue;
        }
        if( n > 0 ) {
            if(readSet.isSet(browserSocket)) {
                auto buf =new byte[8192];
                auto nr=browserSocket.receive(buf);
                if(nr <= 0) {
                    browserSocket.close();
                    proxySocket.close();
                    return;
                }
                buf = buf[0 .. nr];
                simpleEncode(buf);
                auto nw=proxySocket.send(buf);
                if(nw != nr) {
                    browserSocket.close();
                    proxySocket.close();
                    return;
                }
            }
            if(readSet.isSet(proxySocket)) {
                auto buf=new byte[8192];
                auto nr=proxySocket.receive(buf);
                if(nr <= 0) {
                    browserSocket.close();
                    proxySocket.close();
                    return;
                }
                buf = buf[0 .. nr];
                simpleDecode(buf);
                auto nw=browserSocket.send(buf);
                if(nw != nr) {
                    browserSocket.close();
                    proxySocket.close();
                    return;
                }
            }
        }

    } 
    
}

TcpSocket connectToServer(in char[] ip, in ushort port) {
    auto addr=new InternetAddress(ip,port);
    auto s=new TcpSocket();
    s.connect(addr);
    s.setKeepAlive(10,10);
    return s;
}

TcpSocket makeServer(ushort port) {
    auto s=new TcpSocket();
    s.setOption(SocketOptionLevel.SOCKET,SocketOption.REUSEADDR,1);
    s.bind(new InternetAddress("0.0.0.0",port));
    s.listen(100);
    return s;
}

bool simpleEncode(byte[] str) {
    foreach(ref c; str) {
        c += 3;
    }
    return true;
}

bool simpleDecode(byte[] str) {
    foreach(ref c; str) {
        c -= 3;
    }
    return true;
}

void usage() {
    writef("-h\t\t\tprint this help and exit.\n");
    writef("-d\t\t\trun as a daemon.\n");
    writef("-p <port>\t\tTCP port number to listen on (default: 11080).\n");
    writef("-H <host>\t\tserver's host address.\n");
    writef("-P <port>\t\tserver's port number.\n");
}
