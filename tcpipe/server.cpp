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
#include <stdlib.h>

using namespace std;

bool dealWithThread(long cfd);
void *sock4aWorker(void *arg);

char err[BUF_SIZE];
char peer_ip[128];
unsigned short peer_port;

void usage() {
    printf("-h\t\t\tprint this help and exit.\n");
    printf("-d\t\t\trun as a daemon.\n");
    printf("-p <port>\t\tTCP port number to listen on (default: 11080).\n");
    printf("-H <host>\t\tserver's host address.\n");
    printf("-P <port>\t\tserver's port number.\n");

}

int main(int argc,char **argv) {
    int port=LISTEN_PORT;
    int ch;
    bool bg=false;
    while((ch = getopt(argc, argv, "P:H:p:dh")) != -1) {
        switch (ch) {
            case 'p':
                port=atoi(optarg);
                break;
            case 'P':
                peer_port=atoi(optarg);
                break;
            case 'H':
    		sprintf(peer_ip,"%s",optarg);
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

    char addr[64];
    sprintf(addr,"%s","0.0.0.0");
    int sfd=anetTcpServer(err,port,addr);
    if(sfd == ANET_ERR) {
        cerr << "main:anetTcpServer: " << err << endl;
        return -1;
    }
    while(true) {
        int cfd=anetTcpAccept(err,sfd,NULL,NULL);
        anetTcpNoDelay(err,cfd);
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
    int         cfd=(long)arg;
    int         nr,nw;
    char        buffer[BUF_SIZE];
    int         peer_fd;
    struct      pollfd pfds[2];
    peer_port=3128;
    anetTcpKeepAlive(err,cfd);

    pfds[0].fd=cfd;
    pfds[0].events=POLLIN;
    pfds[0].revents=0;
    int tp=0;
    while (true) {
        tp=poll(pfds,1,-1);
        if(tp == 1) {
            peer_fd=anetTcpConnect(err,peer_ip,peer_port);
            break;
        } else {
            continue;
        }
    }

    if(peer_fd == ANET_ERR) {
        cerr << "sock4aWorker:connect_to_server: " << err << endl;
        close(cfd);
        return NULL;
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
                    if(nr < 0) {
                        cerr << "sock4aWorker:read_from_client: "; perror("");
                    }
                    close(pfds[0].fd);
                    close(pfds[1].fd);
                    return NULL;
                }
                simpleDecode(buffer,nr);
                if((nw=anetWrite(pfds[1].fd,buffer,nr)) != nr) {
                    cerr << "sock4aWorker:write_to_server: "; perror("");
                    close(pfds[1].fd);
                    close(pfds[0].fd);
                    return NULL;
                }
            }
            if(pfds[1].revents != 0) {
                if((nr=read(pfds[1].fd,buffer,BUF_SIZE)) <= 0) {
                    if(nr < 0) {
                        cerr << "sock4aWorker:read_from_server: "; perror("");
                    };
                    close(pfds[1].fd);
                    close(pfds[0].fd);
                    return NULL;
                }
                simpleEncode(buffer,nr);
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
