import sys, traceback, Ice

Ice.loadSlice('Balancer.ice')
import Balancer

try:
    communicator = Ice.initialize(sys.argv)
    balancer = Balancer.myBalancerPrx.checkedCast(communicator.stringToProxy("balancer:tcp -p 9999"))
    sql="insert into /*table*/ (ip) values ( '/*val*/')"
    for i in range(1,10) :
        _sql=sql.replace("/*val*/",str(i))
        res=balancer.sqlInsert(_sql,"test.ip",str(i))
        print res
    sql="update /*table*/ set ip=ip*20 where ip=/*val*/"
    for i in range(1,10) :
        _sql=sql.replace("/*val*/",str(i))
        res=balancer.sqlUpdate(_sql,"test.ip",str(i))
        print res
    sql="select * from /*table*/ where ip=/*val*/"
    for i in range(1,10) :
        _sql=sql.replace("/*val*/",str(i*20))
        res=balancer.sqlSelect(_sql,"test.ip",str(i),-1)
        print res
    sql="delete from /*table*/ where ip=/*val*/"
    for i in range(1,10) :
        _sql=sql.replace("/*val*/",str(i*20))
        res=balancer.sqlDelete(_sql,"test.ip",str(i))
        print res 
    communicator.destroy()
except:
    traceback.print_exc()
    sys.exit(1)
