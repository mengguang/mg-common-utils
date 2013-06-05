#!/bin/env python
from twisted.internet.protocol import Factory
from twisted.internet import reactor, protocol

def simpleEncode(data):
    ret=""
    for c in data:
        x=ord(c)+3
        if x > 256:
            x=x-256
        ret += chr(x)
    return ret

def simpleDecode(data):
    ret=""
    for c in data:
        x=ord(c)-3
        if x < 0:
            x=x+256
        ret += chr(x)
    return ret

class BrowserProtocol(protocol.Protocol):
    def __init__(self,factory,reactor=reactor):
        self.factory=factory
        self.reactor=reactor
        self.buf=""
    
    def connectionMade(self):
        self.factory.numConnections+=1
        self.child=SocksFactory(self)
        reactor.connectTCP('www.okbuy.com',21080,self.child)

    def dataReceived(self,data):
        data=simpleEncode(data)
        if not self.child.pro.transport:
            self.buf += data
        else:
            self.child.pro.transport.write(data)
    
    def connectionLost(self,reason):
        self.factory.numConnections-=1
        self.child.pro.transport.loseConnection()
        

class BrowserFactory(Factory):
    numConnections=0;
    
    def __init__(self):
        pass

    def buildProtocol(self,addr):
        return BrowserProtocol(self)

class SocksProtocol(protocol.Protocol):
    def __init__(self,factory,parent):
        self.factory=factory
        self.parent=parent
    
    def connectionMade(self):
        self.transport.write(self.parent.buf)
    
    def dataReceived(self,data):
        data=simpleDecode(data)
        self.parent.transport.write(data);

    def connectionLost(self,reason):
        self.parent.transport.loseConnection()

class SocksFactory(protocol.ClientFactory):

    def __init__(self,parent):
        self.parent=parent
        self.pro=SocksProtocol(self,parent)

    def buildProtocol(self,addr):
        return self.pro
    
    def clientConnectionFailed(self,connector,reason):
        print "connection failed: ", reason.getErrorMessage()

    def clientConnectionLost(self,connector,reason):
        print "connection lost:", reason.getErrorMessage()

reactor.listenTCP(21080,BrowserFactory())
reactor.run()

