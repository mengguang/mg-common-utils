#include <time.h>
#define READ_BUF_LEN 1024
#define WRITE_BUF_LEN 1024

struct RedisClient {
	int fd;
	char *rbuf;
	char *wbuf;
	int rlen;
	int wlen;
	int wpos;
	time_t lastinteraction;
};
typedef struct RedisClient redisClient;

typedef void redisCommandInit(void **privptr);
typedef void redisCommandProc(redisClient *c,void **privptr);
typedef void redisCommandDeinit(void **privptr);
typedef int processRequestBuffer(char *rbuf,int rlen);
