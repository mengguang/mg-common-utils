import asyncell
import socket

class EchoHandler(asyncell.dispatcher):

    def __init__(self,sock):
        asyncell.dispatcher.__init__(self,sock)
        self.event_add_read()
        self.out_buf=''
    def handle_read(self):
        data=self.recv(8192)
        if data == '':
            return
        self.out_buf = self.out_buf + data
        self.event_mod_read_write()
    def handle_write(self):
        if len(self.out_buf) == 0:
            self.event_mod_read()
            return
        else:
            num_sent = 0
            num_sent = self.send(self.out_buf)
            self.out_buf = self.out_buf[num_sent:]
            if len(self.out_buf) == 0:
                self.event_mod_read()
            else:
                self.event_mod_read_write()
            return
        

class EchoServer(asyncell.dispatcher):

    def __init__(self, host, port):
        asyncell.dispatcher.__init__(self)
        self.create_socket(socket.AF_INET, socket.SOCK_STREAM)
        self.set_reuse_addr()
        self.bind((host, port))
        self.listen(5)
        self.event_add_read()

    def handle_accept(self):
        pair = self.accept()
        if pair is None:
            pass
        else:
            sock, addr = pair
            self.log('Incoming connection from %s' % repr(addr))
            handler = EchoHandler(sock)

server = EchoServer('localhost', 20002)
try:
    asyncell.loop()
except KeyboardInterrupt as e:
    asyncell.close_all()
    