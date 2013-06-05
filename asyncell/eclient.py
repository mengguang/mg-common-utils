import socket
s=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
s.connect(("127.0.0.1",20002))
for i in xrange(1000000):
    if i % 1000 == 0:
        print i
    s.send("xxxx")
    s.recv(1024)

