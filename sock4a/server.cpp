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

void usage() {
    printf("-h\t\t\tprint this help and exit.\n");
    printf("-d\t\t\trun as a daemon.\n");
    printf("-p <port>\t\tTCP port number to listen on (default: 11080).\n");
}

int main(int argc,char **argv) {
    int port=LISTEN_PORT;
    int ch;
    bool bg=false;
    while((ch = getopt(argc, argv, "p:dh")) != -1) {
        switch (ch) {
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
    unsigned    short peer_port;
    char        peer_ip[128];
    int         peer_fd;
    struct      pollfd pfds[2];

    anetTcpKeepAlive(err,cfd);
    nr=read(cfd,buffer,BUF_SIZE);
    if(nr < 0 ) {
        cerr << "sock4aWorker:read_from_client: "; perror("");
        close(cfd);
        return NULL;
    }
    if(nr == 0) {
        close(cfd);
        return NULL;
    }
    if(nr <= 8) {
        cerr << "sock4aWorker:read: Bad sock4a header length: " << nr << endl;
        close(cfd);
        return NULL;
    }
    simpleDecode(buffer,nr);
    if(buffer[0] != 0x04 || buffer[1] != 0x01 ) {
        close(cfd);
        cerr << "sock4aWroker:read: Bad sock4a header version." << endl;
        return NULL;
    }
    peer_port=((unsigned char)buffer[2] << 8) + (unsigned char)buffer[3];
    if(buffer[4] == 0 && buffer[4] == 0 && buffer[6] == 0) {
        char *p=strchr(buffer+8,0);
        if(p == NULL ) {
            close(cfd);
            cerr << "sock4aWorker:strchr: Bad sock4a IP address." << endl;
            return NULL;
        } else {
            strcpy(peer_ip,p+1);
        }
    } else {
        sprintf(peer_ip,"%u.%u.%u.%u",(unsigned char)buffer[4],
                                      (unsigned char)buffer[5],
                                      (unsigned char)buffer[6],
                                      (unsigned char)buffer[7]);
    }
    peer_fd=anetTcpConnect(err,peer_ip,peer_port);
    if(peer_fd == ANET_ERR) {
        cerr << "sock4aWorker:connect_to_server: " << err << endl;
        close(cfd);
        return NULL;
    }
    buffer[0]=0x00;
    buffer[1]=0x5a;
    buffer[2]=0x00;
    buffer[3]=0x00;
    buffer[4]=0x00;
    buffer[5]=0x00;
    buffer[6]=0x00;
    buffer[7]=0x00;
    simpleEncode(buffer,8);
    nw=anetWrite(cfd,buffer,8);
    if(nw != 8) {
        cerr << "sock4aWorker:write_to_client: " ; perror("");
        close(cfd);
        close(peer_fd);
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
