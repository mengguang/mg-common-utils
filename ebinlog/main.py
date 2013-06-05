import sys
import os
import string
import time
import signal
import logging
import threading

from Queue import Queue
from optparse import OptionParser
from BinlogListener import *

def sighandler(signum,frame):
    pass

if __name__ == "__main__":
    #signal.signal(signal.SIGTERM,sighandler)
    #signal.signal(signal.SIGHUP,sighandler)

    try:
        parser = OptionParser()
        parser.add_option("-m","--module-path",dest = "module_path")
        parser.add_option("-l","--log-path",dest = "log_path")
        parser.add_option("-s","--queue-size",dest = "queue_size",default=2000)
    except Exception as e:
        print e.args
        sys.exit(0)
    (options,args) = parser.parse_args()

    logger = logging.getLogger()
    if options.log_path == None:
        handler=logging.StreamHandler(sys.stderr)
    else:
        handler = logging.FileHandler(options.log_path)
    formatter = logging.Formatter('%(asctime)s %(levelname)s %(message)s')
    handler.setFormatter(formatter)
    logger.addHandler(handler)
    logger.setLevel(logging.INFO)
    logger.info("main thread started.")

    i = string.rfind(options.module_path,'/')
    module_path = options.module_path[:i+1] 
    j = string.rfind(options.module_path,'.')
    module_name = options.module_path[i+1:j]
    sys.path.append(module_path)
    business = __import__(module_name) 

    master_info = business.get_master_info()
    if len(master_info) != 8:
        logger.error("Get master info from bussiness module failed.")
        sys.exit(0)
    
    msg_queue = Queue(options.queue_size)
    listenerThread = threading.Thread(target = BinlogListenerThread,args = (master_info,msg_queue,logger))
    listenerThread.start();

    business.init(logger)
    while True:
        rows=msg_queue.get();
        if rows == 'STOP':
            business.process('STOP')
            break;
        else :
            for row in rows:
                business.process(row)
    
    business.deinit()
    sys.exit(0)
    #listenerThread.join()
