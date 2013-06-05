#include <iostream>
#include <Poco/Net/TCPServer.h>
#include <Poco/Net/TCPServerConnectionFactory.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/TCPServerParams.h>
#include <Poco/Net/Socket.h>
#include <Poco/ThreadPool.h>
#include <unistd.h>

using namespace std;
using namespace Poco;
using namespace Poco::Net;

class SocksConnection : public TCPServerConnection {
private:
    char buf[8192];
    void simpleDecode(int len) {
        for(;len >0;len--) {
            buf[len-1] -=3;
        }
    }
    void simpleEncode(int len) {
        for(;len >0;len--) {
            buf[len-1] +=3;
        }
    }
public:
    SocksConnection(const StreamSocket &s) : TCPServerConnection(s) {
    }
    void run() {
        cerr << "new connection." << endl;
        StreamSocket &browser=socket();
        StreamSocket socks;
        SocketAddress addr("",21080);
        socks.connect(addr);
        Socket::SocketList sl;
        Socket::SocketList el;
        int sr=0;
        Timespan timeout(60,0);
        while(true) {
            sl.clear();
            sl.push_back(browser);
            sl.push_back(socks);
            sr=Socket::select(sl,el,el,timeout);
            if(sr <= 0) {
                continue;
            }
            if(sl[0] == browser) {
                int nr=browser.receiveBytes(&buf,8192);
                if(nr <= 0) {
                    browser.close();
                    socks.close();
                    cerr << "browser connection closed." << endl;
                    return;
                }
                simpleEncode(nr);
                int nw=socks.sendBytes(&buf,nr);
                if(nw != nr) {
                    browser.close();
                    socks.close();
                    cerr << "socks server write error." << endl;
                    return;
                }
            } else {
                int nr=socks.receiveBytes(&buf,8192);
                if(nr <= 0) {
                    browser.close();
                    socks.close();
                    cerr << "socks server connection closed." << endl;
                    return;
                }
                simpleDecode(nr);
                int nw=browser.sendBytes(&buf,nr);
                if(nw != nr) {
                    browser.close();
                    socks.close();
                    cerr << "browser write error." << endl;
                    return;
                }
            }
        }
    }
};

int main(int argc,char **argv) {
    ServerSocket ss(21080);
    ss.setReuseAddress(true);
    ThreadPool pool(16,1024,60,0);
    
    TCPServer ts(new TCPServerConnectionFactoryImpl<SocksConnection>(),pool,ss);
    ts.start();
    while(true) {
        sleep(5);
        cerr << "current connections: " << ts.currentConnections() << endl;
        cerr << "current queued connections: " << ts.queuedConnections() << endl;
        cerr << "current threads: " << ts.currentThreads() << endl;
    }
    return 0;
}
