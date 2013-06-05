#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <errno.h>
#include "redis.h"
#include "anet.h"

#define SERVER_TIME_OUT 1000 // ms
#define CONN_ERR "-ERR: Connection error\r\n"
#define CONN_ERR_LEN 24

static inline int timeout(struct timeval begin) {
    struct timeval now;
    gettimeofday(&now,NULL);
    int passed;
    passed=(now.tv_sec - begin.tv_sec) * 1000;
    passed+=(now.tv_usec - begin.tv_usec) / 1000;
    return passed;
}

static inline int connectRedis() {
    int fd=anetTcpConnect(NULL, "10.210.74.152", 6379);
    return fd;
}

/*
return -1 if protocol error;
return 0 if we need more data;
return 1 if the request if full and valid

request example:

*3\r\n
$3\r\n
SET\r\n
$5\r\n
mykey\r\n
$7\r\n
myvalue\r\n

*/

int _processRequestBuffer(char *rbuf,int rlen) {
    if( rlen < 3 || rbuf[rlen-2] != '\r'|| rbuf[rlen-1] != '\n' ) {
        return 0;
    }
    if( rbuf[0] != '*' ) { //possibly old protocol command
        if(strcasecmp(rbuf,"quit\r\n") == 0) {
            return -2;
        }
        if(strcasecmp(rbuf,"exit\r\n") == 0) {
            exit(1);
        }
        return 1;
    }
    char *op=rbuf;
    char slen[11];
    int argc,ilen;
    memset(slen,0,11);
    rbuf++; // '*'
    memset(slen,0,11);
    char *p=strstr(rbuf,"\r\n");
    strncpy(slen,rbuf,p-rbuf);
    rbuf=p;
    rbuf+=2; // "\r\n"
    argc=atoi(slen);
    if(argc <= 0 || argc > 1024) {
        return -1;
    }
    if(rbuf - op >= rlen) {
        return 0; // we need more data.
    }
    for(; argc > 0; argc--) {
        if(rbuf[0] != '$') {
            return -1;
        }
        rbuf++; // '$'
        memset(slen,0,11);
        p=strstr(rbuf,"\r\n");
        strncpy(slen,rbuf,p-rbuf);
        rbuf=p;
        rbuf+=2; //"\r\n"
        ilen=atoi(slen);
        if(ilen == -1) {
            ilen=0;
        } else {
            rbuf+=ilen+2; // data and "\r\n"
        }
        if(rbuf - op > rlen) {
            return 0; // we need the next bulk.
        }
        if(rbuf - op == rlen && argc != 1) { // not the last bulk
            return 0;
        }
    }
    if(rbuf - op != rlen) {
        return -1;
    } else {
        return 1;
    }
    return -1;   
}

/*
return -1 if protocol error;
return 0 if we need more data;
return 1 if the request if full and valid

request example:

*3\r\n
$3\r\n
SET\r\n
$5\r\n
mykey\r\n
$7\r\n
myvalue\r\n

*/

static inline int validSingleLineResponseBuffer(char *rbuf,int rlen) {
    if( rbuf[0] != '+' ) {
        return -1;
    }
    if( rlen < 3 || rbuf[rlen-2] != '\r'|| rbuf[rlen-1] != '\n' ) {
        return 0;
    }
    //rbuf++; // '+'
    char *p=strstr(rbuf,"\r\n");
    if(p - rbuf + 2 != rlen) {
        return -1;
    } else {
        return 1;
    }
    return -1;
}

static inline int validErrorMsgResponseBuffer(char *rbuf,int rlen) {
    if( rbuf[0] != '-' ) {
        return -1;
    }
    if( rlen < 3 || rbuf[rlen-2] != '\r'|| rbuf[rlen-1] != '\n' ) {
        return 0;
    }
    //rbuf++; // '-'
    char *p=strstr(rbuf,"\r\n");
    if(p - rbuf + 2 != rlen) {
        return -1;
    } else {
        return 1;
    }
    return -1;
}

static inline int validIntegerReplyResponseBuffer(char *rbuf,int rlen) {
    if( rbuf[0] != ':' ) {
        return -1;
    }
    if( rlen < 3 || rbuf[rlen-2] != '\r'|| rbuf[rlen-1] != '\n' ) {
        return 0;
    }
    //rbuf++; // ':'
    char *p=strstr(rbuf,"\r\n");
    if(p - rbuf + 2 != rlen) {
        return -1;
    } else {
        return 1;
    }
    return -1;
}

static inline int validBulkReplyResponseBuffer(char *rbuf,int rlen) {
    char *op=rbuf;
    if(rbuf[0] != '$') {
        return -1;
    }
    if( rlen < 3 || rbuf[rlen-2] != '\r'|| rbuf[rlen-1] != '\n' ) {
        return 0;
    }
    rbuf++; // '$'
    char slen[11];
    int ilen;
    memset(slen,0,11);
    char *p=strstr(rbuf,"\r\n");
    strncpy(slen,rbuf,p-rbuf);
    rbuf=p;
    rbuf+=2; //"\r\n"
    ilen=atoi(slen);
    if(ilen == -1) {
        rbuf+=0;
    } else {
        rbuf+=ilen+2; // data and "\r\n"
    }
    
    if(rbuf - op > rlen) {
        return 0; // we need the next bulk.
    }
    if(rbuf - op == rlen) {
        return 1;
    }
    if(rbuf - op < rlen) {
        return -1;
    }
}

/*
*2\r\n$-1\r\n$3\r\n456\r\n
*/
static inline int validMultiBuckResponseBuffer(char *rbuf,int rlen) {
    char *op=rbuf;
    if( rbuf[0] != '*' ) {
        return -1;
    }
    if( rlen < 3 || rbuf[rlen-2] != '\r'|| rbuf[rlen-1] != '\n' ) {
        return 0;
    }
    rbuf++; // '*'
    char slen[11];
    int argc,ilen;
    memset(slen,0,11);
    char *p=strstr(rbuf,"\r\n");
    strncpy(slen,rbuf,p-rbuf);
    rbuf=p;
    rbuf+=2; // "\r\n"
    argc=atoi(slen);
    if(argc < 0 || argc > 1024) {
        return -1;
    }
    if( argc == 0 && rbuf - op == rlen) {
        return 1; // we need more data.
    }
    if( argc != 0 && rbuf - op == rlen ) {
        return 0;
    }
    for(; argc > 0; argc--) {
        if(rbuf[0] != '$') {
            return -1;
        }
        rbuf++; // '$'
        memset(slen,0,11);
        p=strstr(rbuf,"\r\n");
        strncpy(slen,rbuf,p-rbuf);
        rbuf=p;
        rbuf+=2; //"\r\n"
        ilen=atoi(slen);
        if(ilen == -1) {
            rbuf+=0;
        } else {
            rbuf+=ilen+2; // data and "\r\n"
        }
        if(rbuf - op > rlen) {
            return 0; // we need the next bulk.
        }
        if(rbuf - op == rlen && argc != 1) { // not the last bulk
            return 0;
        }
    }
    if(rbuf - op != rlen) {
        return -1;
    } else {
        return 1;
    }
    return -1;   
}

int _processResponseBuffer(char *rbuf,int rlen) {
    char s=rbuf[0];
    if(s == '+') {
        return validSingleLineResponseBuffer(rbuf,rlen);
    }
    if(s == '-') {
        return validErrorMsgResponseBuffer(rbuf,rlen);
    }
    if(s == ':') {
        return validIntegerReplyResponseBuffer(rbuf,rlen);
    }
    if(s == '$') {
        return validBulkReplyResponseBuffer(rbuf,rlen);
    }
    if(s == '*') {
        return validMultiBuckResponseBuffer(rbuf,rlen);
    }
    return -1;

}

void _redisCommandInit(void **privptr) {
    int *fd=calloc(sizeof(int),1);
    *fd=connectRedis();
    *privptr=(void *)fd;
    return;
}

static inline void setClientError(redisClient *c) {
    c->wbuf=realloc(c->wbuf,CONN_ERR_LEN+1);
    if(!c->wbuf) {
        exit(-1);
    }
    memset(c->wbuf,0,CONN_ERR_LEN+1);
    strcpy(c->wbuf,CONN_ERR);
    c->wlen=CONN_ERR_LEN;
    return;
}

void _redisCommandProc(redisClient *c,void **privptr) {
    int *fd,err,ntry=0,v;
    int nwrite,nread,rc;
    struct pollfd pfd;
    struct timeval begin;
    char *buf;

again:
    ntry++;
    fd=(int *)(*privptr);
    if(*fd == -1) {
        *fd=connectRedis();
        if(*fd == -1) {
            setClientError(c);
            return;
        }
    }
    pfd.fd=*fd;
    pfd.events = POLLOUT;
    pfd.revents = 0;
    gettimeofday(&begin,NULL);
    nwrite=0;
    err=1;
    while(poll(&pfd,1,SERVER_TIME_OUT/10) > 0){
        rc=write(*fd, c->rbuf + nwrite, c->rlen - nwrite);
        if(timeout(begin) >= SERVER_TIME_OUT) {
            err=1;
            break;
        }
        if(rc > 0) {
            nwrite+=rc;
            if(nwrite == c->rlen) {
                err=0;
                break;
            } else {
                continue;
            }
        }
        if( rc == -1 ) {
            if( errno == EAGAIN ) {
                continue;
            } else {
                err=2;
                break;
            }
        }
    }
    if(err == 2 ) {
        if(ntry < 2) {
            close(*fd);
            *fd=-1;
            goto again;
        } else {
            err = 1;
        }
    }
    if(err == 1) {
        close(*fd);
        *fd=-1;
        setClientError(c);
        return;
    }

    pfd.events = POLLIN;
    pfd.revents = 0;
    gettimeofday(&begin,NULL);
    err=1;
    while(poll(&pfd,1,SERVER_TIME_OUT/10) > 0) {
        if(timeout(begin) >= SERVER_TIME_OUT) {
            err=1;
            break;
        }
        c->wbuf=realloc(c->wbuf,c->wlen+WRITE_BUF_LEN);
        if(!c->wbuf) {
            exit(-1);
        }
        buf=c->wbuf+c->wlen;
        memset(buf,0,WRITE_BUF_LEN);
        rc = read(*fd, buf, WRITE_BUF_LEN);
        if (rc == -1) {
            if (errno == EAGAIN) {
                nread = 0;
                continue;
            } else {
                err=2;
                break;
            }
        }
        if (rc == 0) {
            err=2;
            break;
        }
        if (rc > 0) {
            c->wlen+=rc;
            v=_processResponseBuffer(c->wbuf,c->wlen);
            if(v == 1) {
                err=0;
                break;
            }
            if(v == 0) {
                continue;
            }
            if(v == -1) {
                err=1;
                break;
            }
        }
    }
    if(err == 2 ) {
        if( ntry < 2) {
            close(*fd);
            *fd=-1;
            if(c->wbuf) {
                free(c->wbuf);
                c->wbuf=NULL;
            }
            c->wlen=0;
            goto again;
        } else {
            err=1;
        }
    }
    if(err == 1) {
        close(*fd);
        *fd=-1;
        setClientError(c);
        return;
    }
    return;
}

void _redisCommandDeinit(void **privptr) {
    int *fd=(int *)*privptr;
    close(*fd);
    free(*privptr);
    return;
}
