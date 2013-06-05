#include <QtCore>
#include <QtNetwork>
#include "pipe.h"
#include "server.h"

int main(int argc,char **argv) {
    QCoreApplication app(argc,argv);
    QTcpServer *server=new QTcpServer();
    server->listen(QHostAddress::Any,21080);
    SocksServer *ss=new SocksServer(server);
    QObject::connect(server,SIGNAL(newConnection()),ss,SLOT(newConnection()));
    app.exec();
}
