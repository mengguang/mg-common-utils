#include <time.h>
#define READ_BUF_LEN 128
#define WRITE_BUF_LEN 128

struct RedisClient {
	int fd;
	char rbuf[READ_BUF_LEN];
	char wbuf[WRITE_BUF_LEN];
	int rlen;
	int wlen;
	int wpos;
	time_t lastinteraction;
};
typedef struct RedisClient redisClient;

typedef void redisCommandInit();
typedef void redisCommandProc(redisClient *c);
typedef void redisCommandDeinit();
