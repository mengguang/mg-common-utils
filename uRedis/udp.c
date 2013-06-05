#include "udp.h"

static void anetSetError2(char *err, const char *fmt, ...) {
    va_list ap;
    if (!err) return;
    va_start(ap, fmt);
    vsnprintf(err, ANET_ERR_LEN, fmt, ap);
    va_end(ap);
}

int headerEncode(char *wbuf,int req,int seq,int ngram) {
    if(req < 0 || req > 65535 || seq < 0 || seq > 65535 || ngram < 0 || ngram > 65535) {
        return -1;
    }
    wbuf[0]=req >> 8;
    wbuf[1]=(req << 8) >> 8;
    wbuf[2]=seq >> 8;
    wbuf[3]=(seq << 8) >> 8;
    wbuf[4]=ngram >> 8;
    wbuf[5]=(ngram << 8) >> 8;
    wbuf[6]=0;
    wbuf[7]=0;
    return 8;
}

int headerDecode(unsigned char *hdr,int *req,int *seq,int *ngram) {
    *req=((int)hdr[0] << 8) + hdr[1];
    *seq=((int)hdr[2] << 8) + hdr[3];
    *ngram=((int)hdr[4] << 8) + hdr[5];
    return 8;
}

void processCommandFromUdpClient(aeEventLoop *el, int fd, void *privdata, int mask) {
    REDIS_NOTUSED(el);
    REDIS_NOTUSED(mask);
    REDIS_NOTUSED(privdata);
    
    struct sockaddr sa;
    
    const int hdr_len=8;
    const int rbuf_len=1024;
    const int wbuf_len=65536;
    const int err_len=1024;
    const int psize=1024;
    const int key_len=1024;
    const int val_len=16384;
    char rbuf[rbuf_len];
    char wbuf[wbuf_len];
    char err[err_len];
    char pbuf[psize];
    char key[key_len];
    char val[val_len];
    memset(rbuf,0,rbuf_len);
    memset(wbuf,0,wbuf_len);
    memset(err,0,err_len);
    memset(pbuf,0,psize);
    memset(key,0,key_len);
    
    int nread=0;
    int nwrite=0;
    int wlen=0;
        
    nread = anetUdpRecvFrom(err,fd,&sa,rbuf,rbuf_len);
    if (nread == ANET_ERR) {
        redisLog(REDIS_WARNING, "Reading from UDP client: %s",err);
        return;
    } else if (nread == 0) {
        redisLog(REDIS_WARNING, "Got an empty UDP request.");
        return;
    } else if(nread <= hdr_len || rbuf[hdr_len] != '*') {
        redisLog(REDIS_WARNING, "Invalid Request 0.");
        return;
    }
    
    int req=0;
    int seq=0;
    int ngram=0;
    
    headerDecode((unsigned char *)rbuf,&req,&seq,&ngram);

    char *start=rbuf + hdr_len + 1;
    char *end=strstr(start,"\r\n");
    int nkeys=atoi(start);
    int vlen;
    if(nkeys < 2 || nkeys > 101) {
        wlen+=sprintf(wbuf,"-Err: Invalid keys number.");
        goto uend;
    }
    start=end + 2;
    if(strncmp(start,"$4\r\nmget\r\n",strlen("$4\r\nmget\r\n")) != 0) {
        wlen +=sprintf(wbuf,"-Err: Invalid request method.");
        goto uend;
    }
    start +=strlen("$4\r\nmget\r\n");
    nkeys--;
    wlen+=snprintf(wbuf+wlen,wbuf_len-wlen,"*%d\r\n",nkeys);
    for(int i=0;i<nkeys;i++) {
        memset(key,0,key_len);
        end=strstr(start,"\r\n");
        start=end+2;
        end=strstr(start,"\r\n");
        if(end - start >= key_len ) {
            wlen +=sprintf(wbuf,"-Err: one of the keys is too long.");
            goto uend;
        }
        memcpy(key,start,end-start);
        start=end+2;
        memset(val,0,val_len);
        vlen=getCommandforUdp(key,val,val_len);
        if(vlen < 0) {
            if(wlen + 5 > wbuf_len) {
                wlen=sprintf(wbuf,"-Err: response is too big.");
                goto uend;
            } else {
                wlen+=snprintf(wbuf+wlen,wbuf_len-wlen,"$-1\r\n");
            }
        } else {
            if(wlen + vlen + 10 > wbuf_len) {
                wlen=sprintf(wbuf,"-Err: response is too big.");
                goto uend;
            } else {
                wlen+=snprintf(wbuf+wlen,wbuf_len-wlen,"$%d\r\n",vlen);
                memcpy(wbuf+wlen,val,vlen);
                wlen+=vlen;
                memcpy(wbuf+wlen,"\r\n",2);
                wlen+=2;
            }
        }
    }
uend:
    ngram=(wlen / (psize - hdr_len)) + 1;
    int wpos;
    int wsize;
    for(seq=0;seq<ngram;seq++) {
        wpos=seq*(psize - hdr_len);
        wsize=wlen-wpos > (psize - hdr_len) ? (psize - hdr_len) : wlen-wpos;
        memset(pbuf,0,psize);
        headerEncode(pbuf,req,seq,ngram);
        memcpy(pbuf+hdr_len,wbuf+wpos,wsize);
        nwrite=anetUdpSendTo(err,fd,&sa,pbuf,wsize + hdr_len);
        if(nwrite == ANET_ERR){
            redisLog(REDIS_WARNING,"Writing to UDP client: %s",err);
            return;
        }
    }
    return;
}

int getCommandforUdp(char *key,char *value,int vlen) {
    robj *o=NULL;
    int vsize;
    robj *ok=createStringObject(key,strlen(key));
    o = lookupKeyRead(&server.db[0], ok);
    decrRefCount(ok);
    if (!o) {
        return REDIS_ERR;
    }
    if (o->type != REDIS_STRING) {
        return REDIS_ERR;
    } else {
        if (o->encoding == REDIS_ENCODING_RAW){
            vsize=stringObjectLen(o);
            memcpy(value,o->ptr,vsize > vlen ? vlen : vsize);
            return vsize;
        }
        if (o->encoding == REDIS_ENCODING_INT) {
            vsize=snprintf(value,vlen,"%ld",(long)(o->ptr));
            return vsize;
        }
        return REDIS_ERR;
    }
}

int anetCreateUdpSocket(char *err) {
    int s, on = 1;
    if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        anetSetError2(err, "creating udp socket: %s", strerror(errno));
        return ANET_ERR;
    }

    /* Make sure connection-intensive things like the redis benckmark
     * will be able to close/open sockets a zillion of times */
    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1) {
        anetSetError2(err, "setsockopt SO_REUSEADDR: %s", strerror(errno));
        return ANET_ERR;
    }
    return s;
}

int anetUdpServer(char *err, int port, char *bindaddr) {
    int s;
    struct sockaddr_in sa;

    if ((s = anetCreateUdpSocket(err)) == ANET_ERR)
        return ANET_ERR;

    memset(&sa,0,sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bindaddr && inet_aton(bindaddr, &sa.sin_addr) == 0) {
        anetSetError2(err, "invalid bind address");
        close(s);
        return ANET_ERR;
    }
    if (bind(s,(struct sockaddr*)&sa,sizeof(sa)) == -1) {
        anetSetError2(err, "bind: %s", strerror(errno));
        close(s);
        return ANET_ERR;
    }
    return s;
}

int anetUdpRecvFrom(char *err, int s, struct sockaddr *addr, void *buf, int buf_len) {
    ssize_t size;
    socklen_t salen=sizeof(struct sockaddr);
    size=recvfrom(s,buf,buf_len,0,addr,&salen);
    if(size == -1) {
        anetSetError2(err, "recvfrom: %s", strerror(errno));
        return ANET_ERR;
    }
    return size;
}

int anetUdpSendTo(char *err, int s, struct sockaddr *addr, void *buf, int len) {
    ssize_t size;
    socklen_t salen=sizeof(struct sockaddr);
    size=sendto(s,buf,len,0,addr,salen);
    if(size == -1) {
        anetSetError2(err, "recvfrom: %s", strerror(errno));
        return ANET_ERR;
    }
    return size;
}
