#ifndef SOCKSSERVER_H
#define SOCKSSERVER_H

#include <QObject>
#include <QtNetwork>

class SocksServer : public QObject
{
    Q_OBJECT
private:
    QTcpServer *server;
public:
    explicit SocksServer(QTcpServer *s,QObject *parent = 0);
    
signals:
    
public slots:
    void newConnection();
};

#endif // SOCKSSERVER_H
