
import sys, traceback, time, Ice
from DB import DB
Ice.loadSlice('Balancer.ice')
import Balancer

class myBalancerI(Balancer.myBalancer):
    def __init__(self):
        self.db=DB()
    def sqlInsert(self, sql, table, hashval, current=None):
        return self.db.sql_insert(sql,table,hashval)
    def sqlDelete(self, sql, table, hashval, current=None):
        return self.db.sql_delete(sql,table,hashval)
    def sqlUpdate(self, sql, table, hashval, current=None):
        return self.db.sql_update(sql,table,hashval)
    def sqlSelect(self, sql, table, hashval, expire, current=None):
        return self.db.sql_select(sql,table,hashval,expire)

class Server(Ice.Application):
    def run(self, args):
        adapter = self.communicator().createObjectAdapter("Balancer")
        adapter.add(myBalancerI(), self.communicator().stringToIdentity("balancer"))
        adapter.activate()
        self.communicator().waitForShutdown()
        return 0

sys.stdout.flush()
app = Server()
sys.exit(app.main(sys.argv,"config.server"))
