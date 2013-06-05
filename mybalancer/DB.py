import memcache
import MySQLdb
import random
import json
from hashlib import md5
import time

class DB :
    def __init__(self) :
        self.tables={
            "test.ip" : 
                {
                    "slice" : 3
                }
            }        
        self.mysqls={
            "test.ip_0" : 
             {
                "10.210.74.152:3306" : 
                    {
                        "host" : "10.210.74.152" , 
                        "port" : 3306 , 
                        "user" : "mg" , 
                        "passwd" : "123qwe" ,
                        "max_conn" : 3
                    },
                "127.0.0.1:3306" : 
                    {
                        "host" : "127.0.0.1" , 
                        "port" : 3306 , 
                        "user" : "root" , 
                        "passwd" : "" ,
                        "max_conn" : 2
                    }
             },
            "test.ip_1" : 
             {
                "10.210.74.152:3306" : 
                    {
                        "host" : "10.210.74.152" , 
                        "port" : 3306 , 
                        "user" : "mg" , 
                        "passwd" : "123qwe" ,
                        "max_conn" : 3
                    },
                "127.0.0.1:3306" : 
                    {
                        "host" : "127.0.0.1" , 
                        "port" : 3306 , 
                        "user" : "root" , 
                        "passwd" : "" ,
                        "max_conn" : 2
                    }
             },
            "test.ip_2" : 
             {
                "10.210.74.152:3306" : 
                    {
                        "host" : "10.210.74.152" , 
                        "port" : 3306 , 
                        "user" : "mg" , 
                        "passwd" : "123qwe" ,
                        "max_conn" : 3
                    },
                "127.0.0.1:3306" : 
                    {
                        "host" : "127.0.0.1" , 
                        "port" : 3306 , 
                        "user" : "root" , 
                        "passwd" : "" ,
                        "max_conn" : 2
                    }
             }
        }
        self.mysql_conn=None
        self.connect_mysqls()
        
        self.memcaches=['127.0.0.1:11211','127.0.0.1:11212']
        self.memcache_conn=None
        self.connect_memcaches()
        
    def connect_memcaches(self) :
        self.memcache_conn=memcache.Client(self.memcaches)
        
    def connect_mysqls(self) :
        self.mysql_conn={}
        for table in self.mysqls.keys() :
            servers=self.mysqls[table]
            self.mysql_conn[table]=[]
            for server in servers.values() :
                for i in range(0,server['max_conn']):
                    try :
                        conn=MySQLdb.connect(host=server['host'],port=server['port'],
                            user=server['user'],passwd=server['passwd'])
                    except:
                        conn=None
                    #print conn
                    if conn != None :
                        self.mysql_conn[table].append(conn)
        #print self.mysql_conn
        
    def pack_sql(self,sql,table,hashval) :
        nslice=self.tables[table]["slice"]
        hashr=memcache.crc32(str(hashval))
        slot=hashr % nslice
        table = table + "_" + str(slot)
        sql=sql.replace("/*table*/",table)
        #print sql
        return table,sql

    def sql_execute(self,sql,table,hashval) :
        mycsr=self.get_cursor(table,hashval)
        mycsr.execute(sql)
        return (mycsr.rowcount,mycsr.fetchall())
        
    def get_cursor(self,table,hashval) :
        conns=self.mysql_conn[table]
        pos=memcache.crc32(str(hashval)) % len(conns)
        conn=conns[pos]
        csr=conn.cursor()
        return csr

    def sql_select(self,sql,table,hashval,expire=1) :
        table,sql=self.pack_sql(sql,table,hashval)
        cachekey=md5(sql).hexdigest()
        lockkey=cachekey + ".lock"
        if expire < 0 :
            return json.dumps(self.sql_execute(sql,table,hashval))
        i=0
        while i < 10:
            cache=self.get_from_cache(cachekey)
            if cache:
                return json.dumps(cache)
            else :
                lock=self.lock_cache(lockkey)
                if lock :
                    res=self.sql_execute(sql,table,hashval)
                    self.save_to_cache(cachekey,res,expire)
                    self.unlock_cache(lockkey)
                    return json.dumps(res)
                else :
                    time.sleep(0.1)
                    i=i+1
        return json.dumps(False)

    def sql_common(self,sql,table,hashval) :
        table,sql=self.pack_sql(sql,table,hashval)
        res=self.sql_execute(sql,table,hashval)
        return json.dumps(res)

    def sql_insert(self,sql,table,hashval) :
        return self.sql_common(sql,table,hashval)
    
    def sql_update(self,sql,table,hashval) :
        return self.sql_common(sql,table,hashval)
    
    def sql_delete(self,sql,table,hashval) :
        return self.sql_common(sql,table,hashval)

    def lock_cache(self,key) :
        return self.memcache_conn.add(key,0,1)
    
    def unlock_cache(self,key) :
        return self.memcache_conn.delete(key)
    
    def get_from_cache(self,key) :
        return self.memcache_conn.get(key)
    
    def save_to_cache(self,key,val,expire=5) :
        return self.memcache_conn.set(key,val,expire)

    def test(self) :
        sql="insert into /*table*/ (ip) values ( '/*val*/')"
        for i in range(1,10) :
            _sql=sql.replace("/*val*/",str(i))
            res=self.sql_insert(_sql,"test.ip",i)
            print res
        sql="update /*table*/ set ip=ip*20 where ip=/*val*/"
        for i in range(1,10) :
            _sql=sql.replace("/*val*/",str(i))
            res=self.sql_update(_sql,"test.ip",i)
            print res
        sql="select * from /*table*/ where ip=/*val*/"
        for i in range(1,10) :
            _sql=sql.replace("/*val*/",str(i*20))
            res=self.sql_select(_sql,"test.ip",i,10)
            print res
        sql="select * from /*table*/ where ip=/*val*/"
        for i in range(1,10) :
            _sql=sql.replace("/*val*/",str(i*20))
            res=self.sql_select(_sql,"test.ip",i,-1)
            print res
        sql="delete from /*table*/ where ip=/*val*/"
        for i in range(1,10) :
            _sql=sql.replace("/*val*/",str(i*20))
            res=self.sql_delete(_sql,"test.ip",i)
            print res

if __name__ == "__main__" :
    db=DB()
    db.test()
    #db.print_mysqls()