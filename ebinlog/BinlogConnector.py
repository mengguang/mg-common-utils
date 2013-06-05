import pymysql
import struct

class BinlogConnector:
    
    def __init__(self,master_info):
        self.__master_info=master_info;  

    def __del__(self):
        if self.__conn != None:
            self.__conn.close()
        
    def connect(self):
        try:
            self.__conn = pymysql.connect(
                        host=self.__master_info['host'],
                        port=self.__master_info['port'],
                        user=self.__master_info['user'],
                        passwd=self.__master_info['password']
                        )    
        except Exception as e:
            print e.args
            self.__conn=None
            print "Connection to mysql failed"
            print self.__master_info
            return False
            
        args = struct.pack('<L',self.__master_info['position'])         #position
        args = args + struct.pack('<H',0)                               #flags
        args = args + struct.pack('<L',self.__master_info['serverid'])  #serverid
        args = args + self.__master_info['name']                        #name (optional)
        COM_BINLOG_DUMP = 18
        self.__conn._execute_command(COM_BINLOG_DUMP, args)
        
    def get_socket_fd(self):
        return self.__conn.socket.fileno() 


