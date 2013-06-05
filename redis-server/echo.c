#include <string.h>
#include <stdio.h>
#include "redis.h"

void _redisCommandInit() {
    return;
}

void _redisCommandProc(redisClient *c) {
    memcpy(c->wbuf,c->rbuf,c->rlen);
    c->wlen=c->rlen;
    return;
}

void _redisCommandDeinit() {
    return;
}
