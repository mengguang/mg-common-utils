import sys
import os
import string
import time
import struct
import signal
import BinlogProcessor

from BinlogConnector import BinlogConnector

class BinlogListener:
    
    def __init__(self,master_info):
        self.__master_info=master_info;
        self.__data = ''

    def start(self):
        self.connector = BinlogConnector(self.__master_info)
        self.connector.connect()
        BinlogProcessor.Init(self.__master_info['name'],self.__master_info['position'],self.__master_info['index'])

    def get_event(self):
        while True:
            Data = BinlogProcessor.Recv(self.connector.get_socket_fd());
            if(Data == -1):
                return;
            if Data == 0:
                raise Exception(0,"Lost Connection.")
            Data = Data[1:]
            self.__data += Data
            Ret = BinlogProcessor.Process(self.__data)
            print Ret
            self.__data = self.__data[Ret[1]:]
            if Ret[0] == 1:
                return Ret[2]

    def end(self):
        BinlogProcessor.DeInit()
        return 

def BinlogListenerThread(master_info,msg_queue,logger):
    current_master_info = master_info
    listener=None
    while True:
        time.sleep(1)
        try:
            listener = BinlogListener(current_master_info);
            listener.start()
        except Exception, e :
            logger.error("listener started failed.")
            logger.error(e.args)
            continue
        while True:
            try:
                rows = listener.get_event()
            except Exception, e:
                logger.error("get_event failed.")
                logger.error(e.args)
                continue
            try:
                msg_queue.put(rows)
                pos=rows[0]['pos']
                name,position,null=pos.split('/')
                current_master_info['name']=name
                current_master_info['position']=position
            except Exception , e:
                logger.error("put message into queue failed.")
                logger.error(e.args)
                continue
    listener.end()
