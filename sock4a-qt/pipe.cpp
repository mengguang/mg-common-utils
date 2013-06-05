#include "pipe.h"

SocketPipe::SocketPipe(QTcpSocket *b,QTcpSocket *s,QObject *parent) :
    QObject(parent)
{
    browser=b;
    socks=s;
}

void SocketPipe::peerDisconnected() {
    browser->close();
    socks->close();
    browser->deleteLater();
    socks->deleteLater();
    deleteLater();
}

void SocketPipe::socksReadyRead() {
    int r=0;
    socksBuf=socks->readAll();
    if(socksBuf.isEmpty()) {
        browser->close();
        socks->close();
        delete browser;
        delete socks;
        return;
    }
    simpleDecode(socksBuf);
    while(!socksBuf.isEmpty()) {
        r=browser->write(socksBuf);
        if(r == -1) {
            browser->close();
            socks->close();
            delete browser;
            delete socks;
            return;
        } else {
            socksBuf=socksBuf.right(socksBuf.size()-r);
        }
    }
}
void SocketPipe::browserReadyRead() {
    int r=0;
    browserBuf=browser->readAll();
    if(browserBuf.isEmpty()) {
        browser->close();
        socks->close();
        delete browser;
        delete socks;
        return;
    }
    simpleEncode(browserBuf);
    while(!browserBuf.isEmpty()) {
        r=socks->write(browserBuf);
        if(r == -1) {
            browser->close();
            socks->close();
            delete browser;
            delete socks;
            return;
        } else {
            browserBuf=browserBuf.right(browserBuf.size()-r);
        }
    }
}
