#include "server.h"
#include "pipe.h"
#include <QtNetwork>

SocksServer::SocksServer(QTcpServer *s,QObject *parent) :
    QObject(parent)
{
    server=s;
}

void SocksServer::newConnection(){
    QTcpSocket *browserSocket=NULL;
    browserSocket=server->nextPendingConnection();
    QTcpSocket *socks=new QTcpSocket();
    socks->connectToHost("www.okbuy.com",21080);
    SocketPipe *pconn = new SocketPipe(browserSocket,socks,this);
    QObject::connect(browserSocket,SIGNAL(readyRead()),pconn,SLOT(browserReadyRead()));
    QObject::connect(socks,SIGNAL(readyRead()),pconn,SLOT(socksReadyRead()));
    QObject::connect(socks,SIGNAL(disconnected()),pconn,SLOT(peerDisconnected()));
    QObject::connect(browserSocket,SIGNAL(disconnected()),pconn,SLOT(peerDisconnected()));
    QObject::connect(socks,SIGNAL(error(QAbstractSocket::SocketError)),pconn,SLOT(peerDisconnected()));
    QObject::connect(browserSocket,SIGNAL(error(QAbstractSocket::SocketError)),pconn,SLOT(peerDisconnected()));
}

