def init(logger):
    logger.info("business started.")

def deinit():
    pass

def process(row):
    print row

def get_master_info():
    master_info={}
    master_info['host']="10.75.17.25"
    master_info['port']=3601
    master_info['user']="redis"
    master_info['password']="redis_1q2w3e4r5t"
    master_info['name']="ker25-bin.000339"
    master_info['position']=4
    master_info['index']=0
    master_info['serverid']=74152
    return master_info

