#ifndef SOCKETPIPE_H
#define SOCKETPIPE_H

#include <QObject>
#include <QtNetwork>

class SocketPipe : public QObject
{
    Q_OBJECT
private:
    QByteArray browserBuf;
    QByteArray socksBuf;
    QTcpSocket *browser;
    QTcpSocket *socks;

public:
    SocketPipe(QTcpSocket *b,QTcpSocket *s,QObject *parent = 0);
    bool simpleEncode(QByteArray &b) {
        for(int i=0;i<b.size();i++) {
            b[i]=b[i]+3;
        }
        return true;
    }

    bool simpleDecode(QByteArray &b) {
        for(int i=0;i<b.size();i++) {
            b[i]=b[i]-3;
        }
        return true;
    }


signals:

public slots:
    void socksReadyRead();
    void browserReadyRead();
    void peerDisconnected();
};

#endif // SOCKETPIPE_H
