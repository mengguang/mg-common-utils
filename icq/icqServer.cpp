#include <Ice/Ice.h>
#include <Ice/Service.h>
#include <icq.h>
#include <cstdlib>
#include <string>
#include <iostream>
#include <map>
#include <haildb.h>

using namespace std;
/*
module MICQ {
    interface icq {
        bool create(string name);
        string insert(string name,string data);
        string get(string name,string pos);
        string getNext(string name,string pos);
        string getLatest(string name);
        string getOldest(string name);
        string truncate(string name);
    };
};
*/

class icqI : public icq {
public:
    icqI(){ 
        this->init();
    };
    ~icqI(){
        this->deinit();
    };
    virtual bool create(string name,const Ice::Current&);
    virtual string insert(string name,string data,const Ice::Current&);
    virtual string get(string name,string pos,const Ice::Current&);
    virtual string getNext(string name,string pos,const Ice::Current&);
    virtual string getLatest(string name,const Ice::Current&);
    virtual string getOldest(string name,const Ice::Current&);
    virtual string truncate(string name,const Ice::Current&);
private:
    int latest;
    virtual bool init();
    virtual bool deinit();
    virtual open_table(string name)
};

bool icqI::init() {

}
bool icqI::deinit() {

}

bool icqI::create(string name,const Ice::Current&) {

}

string insert(string name,string data,const Ice::Current&) {
    
}


class IcqService:public Ice::Service {
protected:
    virtual bool start(int,char **,int&);
private:
    Ice::ObjectAdapterPtr _adapter;
};

bool IcqService::start(int agrc,char **argv,int& status) {
    Ice::CommunicatorPtr ic;
    ic=Ice::initialize();
    _adapter=ic->createObjectAdapterWithEndpoints("icqAdapter","default -p 10000");
    _adapter->add(new icqI(10),ic->stringToIdentity("icq"));
    _adapter->activate();
    status=EXIT_SUCCESS;
    return true;
}

int main(int argc,char **argv){
    IcqService svc;
    svc.main(argc,argv);
    return 0;
}
