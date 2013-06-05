#include <iostream>
#include <string.h>
#include <stdio.h>
#include <map>
#include <anet.h>
#include <poll.h>
#include <pthread.h>
#include <queue>
#include <signal.h>
#include <lib.h>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

char err[BUF_SIZE];

bool dealWithThread(long cfd);
void *sock4aWorker(void *arg);
void *connection_factory(void *arg);
int connectToServer();
bool deQueue(int &fd);

queue<int> clientQueue;
pthread_mutex_t qlock=PTHREAD_MUTEX_INITIALIZER;
char server_host[64];
int  server_port=LISTEN_PORT;

void *connection_factory(void *arg) {
    int fd;
    int nc=0;
    while(true) {
        pthread_mutex_lock(&qlock);
        nc=clientQueue.size();
        pthread_mutex_unlock(&qlock);
        if (nc < 20) {
            fd=connectToServer();
            if(fd == ANET_ERR) {
                cerr << "Can not connect to server." << endl;
                usleep(200000); // 0.2s
                continue;
            } else {
                anetTcpKeepAlive(err,fd);
                pthread_mutex_lock(&qlock);
                clientQueue.push(fd);
                nc=clientQueue.size();
                pthread_mutex_unlock(&qlock);
                cerr << "connection enQueue OK. now Queue size : " << nc << endl;
            }
        } else {
            usleep(50000); // 0.05s
        }
    }
    return NULL;
}

bool deQueue(int &fd) {
    pthread_mutex_lock(&qlock);
    if(clientQueue.empty()) {
        pthread_mutex_unlock(&qlock);
        fd=connectToServer();
    } else {
        fd=clientQueue.front();
        clientQueue.pop();
        pthread_mutex_unlock(&qlock);
    }
    return true;
}

int connectToServer() {
    if(server_host == NULL) {
        return ANET_ERR;
    }
    int fd=anetTcpConnect(err,server_host,server_port);
    anetTcpNoDelay(err,fd);
    return fd;
}

void usage() {
    printf("-h\t\t\tprint this help and exit.\n");
    printf("-d\t\t\trun as a daemon.\n");
    printf("-p <port>\t\tTCP port number to listen on (default: 11080).\n");
    printf("-H <host>\t\tserver's host address.\n");
    printf("-P <port>\t\tserver's port number.\n");  
}

int main(int argc,char **argv) {

    int  ch;
    bool bg=false;
    int  port=LISTEN_PORT;
    char host[64];
    snprintf(host,63,"%s","0.0.0.0");
    snprintf(server_host,63,"%s","106.187.41.101");
    while((ch = getopt(argc, argv, "H:P:p:dh")) != -1) {
        switch (ch) {
            case 'H':
                snprintf(server_host,63,"%s",optarg);
                break;
            case 'P':
                server_port=atoi(optarg);
                break;
            case 'p':
                port=atoi(optarg);
                break;
            case 'd':
                bg=true;
                break;
            case 'h':
                usage();
                exit(0);
            default:
                break;
        }
    }
    
    if(bg) {
        daemon(0,0);
    }
    
    signal(SIGPIPE, SIG_IGN);    
    int sfd=anetTcpServer(err,port,host);
    if(sfd == ANET_ERR) {
        cerr << "main:anetTcpServer: " <<  err << endl;
        return -1;
    }
    pthread_t pid;
    pthread_create(&pid,NULL,connection_factory,NULL);
    while(true) {
        int cfd=anetTcpAccept(err,sfd,NULL,NULL);
        if(cfd == ANET_ERR) {
            cerr << "main:anetTcpAccept: " << err << endl;
            continue;
        }
        dealWithThread(cfd);
   }
}

bool dealWithThread(long cfd) {
    pthread_t pid;
    pthread_create(&pid,NULL,sock4aWorker,(void *)cfd);
    return true;
}

void *sock4aWorker(void *arg) {
    
    pthread_detach(pthread_self());
    
    int     cfd=(long)arg;
    int     nr,nw;
    int     peer_fd;
    char    buffer[BUF_SIZE];
    struct  pollfd pfds[2];

    deQueue(peer_fd);
    pfds[0].fd=peer_fd;
    pfds[0].events=POLLIN;
    pfds[0].revents=0;
    int tr=poll(pfds,1,1);
    if(tr > 0) {
        cerr << "sock4aWorker:deQueue peer_fd has problem read." << endl;
        peer_fd=connectToServer();
        if(peer_fd == ANET_ERR) {
            cerr << "sock4aWorker:connect_to_server: " << err << endl;
            close(cfd);
            return NULL;
        }
    }

    pfds[0].fd=cfd;
    pfds[0].events=POLLIN;
    pfds[0].revents=0;
    pfds[1].fd=peer_fd;
    pfds[1].events=POLLIN;
    pfds[1].revents=0;
    while(true) {
        int pr=poll(pfds,2,60000);
        if(pr == 0) continue;
        if(pr == -1) {
            continue;
        }
        if(pr >= 1) {
            if(pfds[0].revents != 0) {
                if((nr=read(pfds[0].fd,buffer,BUF_SIZE)) <= 0) {
                    if(nr < 0 ) {
                        cerr << "sock4aWroker:read_from_client: "; perror("");
                    }
                    close(pfds[0].fd);
                    close(pfds[1].fd);
                    return NULL;
                }
                simpleEncode(buffer,nr);
                if((nw=anetWrite(pfds[1].fd,buffer,nr)) != nr) {
                    cerr << "sock4aWorker:write_to_server: "; perror("");
                    close(pfds[0].fd);
                    close(pfds[1].fd);
                    return NULL;
                }
            }
            if(pfds[1].revents != 0) {
                if((nr=read(pfds[1].fd,buffer,BUF_SIZE)) <= 0) {
                    if(nr < 0 ) {
                        cerr << "sock4aWorker:read_from_server: "; perror("");
                    }
                    close(pfds[0].fd);
                    close(pfds[1].fd);
                    return NULL;
                }
                simpleDecode(buffer,nr);
                if((nw=anetWrite(pfds[0].fd,buffer,nr)) != nr) {
                    cerr << "sock4aWorker:write_to_client: "; perror("");
                    close(pfds[0].fd);
                    close(pfds[1].fd);
                    return NULL;
                }
            }
            pfds[0].revents=0;
            pfds[1].revents=0;
        }
    }
    return NULL;
}

