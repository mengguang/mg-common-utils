#define _XOPEN_SOURCE 700
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/uio.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <dlfcn.h>

#include "ae.h"
#include "anet.h"

#define REDIS_VERSION "2.0.4"

/* Error codes */
#define REDIS_OK                0
#define REDIS_ERR               -1

/* Static server configuration */
#define REDIS_SERVERPORT        6376    /* TCP port */
#define REDIS_MAXIDLETIME       (60*5)  /* default client timeout */

/* Log levels */
#define REDIS_DEBUG 0
#define REDIS_VERBOSE 1
#define REDIS_NOTICE 2
#define REDIS_WARNING 3
#define REDIS_FATAL 4

/* Anti-warning macro... */
#define REDIS_NOTUSED(V) ((void) V)
#define redisAssert assert

/* Global server state structure */
struct RedisServer {
    pthread_t mainthread;
    unsigned int clients;
    int port;
    int fd;
    char neterr[ANET_ERR_LEN];
    aeEventLoop *el;
    time_t stat_starttime;         /* server start time */
    long long stat_numcommands;    /* number of processed commands */
    long long stat_numconnections; /* number of connections received */
    int verbosity;
    int maxidletime;
    int daemonize;
    char *pidfile;
    char *logfile;
    char *bindaddr;
    unsigned int maxclients;
    time_t unixtime;    /* Unix time sampled every second. */
    FILE *devnull;
};
typedef struct RedisServer redisServer;

#define READ_BUF_LEN 64
#define WRITE_BUF_LEN 64
#define MAX_READ_BUF_LEN 128

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

static redisClient *createClient(int fd);
static void redisLog(int level, const char *fmt, ...);
static void acceptHandler(aeEventLoop *el, int fd, void *privdata, int mask);
static void readQueryFromClient(aeEventLoop *el, int fd, void *privdata, int mask);
static void processInputBuffer(redisClient *c);
static void initServerConfig() ;
static void initServer();
static void setupSigSegvAction(void);
static void version() ;
static void usage();
static void daemonize(void);
static void sendReplyToClient(aeEventLoop *el, int fd, void *privdata, int mask);
static void sigtermHandler(int sig);
static void segvHandler(int sig, siginfo_t *info, void *secret);
static void freeClient(redisClient *c);
static void resetClient(redisClient *c);
static void oom(const char *msg);
unsigned long long get_uuid();
int ProcessRequestBuffer(redisClient *c);

unsigned long long mid;
unsigned long long sid;

static redisServer server;

static void redisLog(int level, const char *fmt, ...) {
    if (level < server.verbosity) return;
    va_list ap;
    FILE *fp;
    char *c = ".-*#";
    char buf[64];
    time_t now;
    fp = (server.logfile == NULL) ? stdout : fopen(server.logfile,"a");
    if (!fp) return;
    va_start(ap, fmt);
    now = time(NULL);
    strftime(buf,64,"%d %b %H:%M:%S",localtime(&now));
    fprintf(fp,"[%d] %s %c ",(int)getpid(),buf,c[level]);
    vfprintf(fp, fmt, ap);
    fprintf(fp,"\n");
    fflush(fp);
    va_end(ap);
    if (server.logfile) fclose(fp);
}

static void oom(const char *msg) {
    redisLog(REDIS_WARNING, "%s: Out of memory\n",msg);
    sleep(1);
    abort();
}

static void acceptHandler(aeEventLoop *el, int fd, void *privdata, int mask) {
    int cport, cfd;
    char cip[128];
    redisClient *c;
    REDIS_NOTUSED(el);
    REDIS_NOTUSED(privdata);
    REDIS_NOTUSED(mask);
    cfd = anetAccept(server.neterr, fd, cip, &cport);
    if (cfd == AE_ERR) {
        if(server.verbosity <= REDIS_VERBOSE )
            redisLog(REDIS_VERBOSE,"Accepting client connection: %s", server.neterr);
        return;
    }
    if(server.verbosity <= REDIS_VERBOSE )
        redisLog(REDIS_VERBOSE,"Accepted %s:%d", cip, cport);
    if ((c = createClient(cfd)) == NULL) {
        redisLog(REDIS_WARNING,"Error allocating resoures for the client");
        close(cfd); /* May be already closed, just ingore errors */
        return;
    }
    if (server.maxclients && server.clients > server.maxclients) {
        char *err = "-ERR max number of clients reached\r\n";
        /* That's a best effort error message, don't check write errors */
        if (write(c->fd,err,strlen(err)) == -1) {
            /* Nothing to do, Just to avoid the warning... */
        }
        freeClient(c);
        return;
    }
    server.stat_numconnections++;
}

static redisClient *createClient(int fd) {
    redisClient *c = calloc(1,sizeof(redisClient));
    if (!c) return NULL;
    anetNonBlock(NULL,fd);
    anetTcpNoDelay(NULL,fd);
    if (aeCreateFileEvent(server.el,fd,AE_READABLE,readQueryFromClient, c) == AE_ERR) {
        close(fd);
        free(c);
        return NULL;
    }
    c->fd = fd;
    server.clients++;
    return c;
}

static void readQueryFromClient(aeEventLoop *el, int fd, void *privdata, int mask) {
    redisClient *c = (redisClient*) privdata;
    char *buf;
    int nread;
    REDIS_NOTUSED(el);
    REDIS_NOTUSED(mask);
    
    c->rbuf=realloc(c->rbuf,c->rlen+READ_BUF_LEN);
    if(!c->rbuf) {
        oom("realloc client read buffer.");
    }
    buf=c->rbuf+c->rlen;
    memset(buf,0,READ_BUF_LEN);
   
    nread = read(fd, buf, READ_BUF_LEN);
    if (nread == -1) {
        if (errno == EAGAIN) {
            nread = 0;
        } else {
            if(server.verbosity <= REDIS_VERBOSE )
                redisLog(REDIS_VERBOSE, "Reading from client: %s",strerror(errno));
            freeClient(c);
            return;
        }
    } else if (nread == 0) {
        if(server.verbosity <= REDIS_VERBOSE )
            redisLog(REDIS_VERBOSE, "Client closed connection");
        freeClient(c);
        return;
    }
    if (nread) {
        c->rlen+=nread;
        if(c->rlen > MAX_READ_BUF_LEN) { //
            freeClient(c);
            return;
        }
        c->lastinteraction = time(NULL);
    } else {
        return;
    }
    processInputBuffer(c);
}

unsigned long long get_uuid() {
    unsigned long long uuid = (unsigned long long)time(NULL);
    uuid <<= 32;
    uuid += mid << 24;
    sid++;
    uuid += sid % (1 << 24);
    return uuid;
}

int ProcessRequestBuffer(redisClient *c) {
    char *rbuf=c->rbuf;
    int rlen=c->rlen;
    char *p=strstr(rbuf,"\r\n");
    if( p == NULL) {
        return 0;
    } else if(p != rbuf+rlen-2 ) {
        return -1;
    }
    if(strcasecmp(rbuf,"quit\r\n") == 0) { // client ask to quit
        return -2;
    }
    if(strncasecmp(rbuf,"get ",4) == 0 ) {
        c->wbuf=calloc(128,1);
        snprintf(c->wbuf,127,"VALUE uuid 0 19\r\n%llu\r\nEND\r\n",get_uuid());
        c->wlen=strlen(c->wbuf);
        return 1;
    }
    return -1;
}

static void processInputBuffer(redisClient *c) {
    int v=ProcessRequestBuffer(c);
    if(v == -1) { // client protocol error
        freeClient(c);
        return;
    }
    if(v == -2) { //QUIT command
        freeClient(c);
    }
    if(v == 0) { //not enough data
        return;
    }
    if(v == 1) { //data is ok,now send reply to client
        aeDeleteFileEvent(server.el,c->fd,AE_READABLE);
        if (aeCreateFileEvent(server.el, c->fd, AE_WRITABLE,
            sendReplyToClient, c) == AE_ERR) return;
    }
}



static void setupSigSegvAction(void) {
    struct sigaction act;
    sigemptyset (&act.sa_mask);
    /* When the SA_SIGINFO flag is set in sa_flags then sa_sigaction
     * is used. Otherwise, sa_handler is used */
    act.sa_flags = SA_NODEFER | SA_ONSTACK | SA_RESETHAND | SA_SIGINFO;
    act.sa_sigaction = segvHandler;
    sigaction (SIGSEGV, &act, NULL);
    sigaction (SIGBUS, &act, NULL);
    sigaction (SIGFPE, &act, NULL);
    sigaction (SIGILL, &act, NULL);
    sigaction (SIGBUS, &act, NULL);

    act.sa_flags = SA_NODEFER | SA_ONSTACK | SA_RESETHAND;
    act.sa_handler = sigtermHandler;
    sigaction (SIGTERM, &act, NULL);
    return;
}

static void sigtermHandler(int sig) {
    REDIS_NOTUSED(sig);
    redisLog(REDIS_WARNING,"SIGTERM received, scheduling shutting down...");
    _exit(0);
}

static void segvHandler(int sig, siginfo_t *info, void *secret) {
    REDIS_NOTUSED(info);
    REDIS_NOTUSED(secret);
    redisLog(REDIS_WARNING,
        "======= Ooops! Redis %s got signal: -%d- =======", REDIS_VERSION, sig);
    _exit(0);
}

static void freeClient(redisClient *c) {
    aeDeleteFileEvent(server.el,c->fd,AE_READABLE);
    aeDeleteFileEvent(server.el,c->fd,AE_WRITABLE);
    close(c->fd);
    if(c->rbuf) {
        free(c->rbuf);
        c->wbuf=NULL;
    }
    if(c->wbuf) {
        free(c->wbuf);
        c->wbuf=NULL;
    }
    if(c) {
        free(c);
        c=NULL;
    }
    server.clients--;
}

static void sendReplyToClient(aeEventLoop *el, int fd, void *privdata, int mask) {
    redisClient *c = privdata;
    int nwritten = 0;
    REDIS_NOTUSED(el);
    REDIS_NOTUSED(mask);
    nwritten = write(fd, c->wbuf + c->wpos, c->wlen - c->wpos);
    if (nwritten == -1) {
        if (errno == EAGAIN) {
            nwritten = 0;
        } else {
            if(server.verbosity <= REDIS_VERBOSE )
                redisLog(REDIS_VERBOSE,
                    "Error writing to client: %s", strerror(errno));
            freeClient(c);
            return;
        }
    }
    if (nwritten > 0) {
    	c->lastinteraction = time(NULL);
    	c->wpos += nwritten;
    }
    if (c->wpos == c->wlen) {
        resetClient(c);
    }
}

static void resetClient(redisClient *c) {
    if(c->rbuf) {
        free(c->rbuf);
        c->rbuf=NULL;
    }
    if(c->wbuf) {
        free(c->wbuf);
        c->wbuf=NULL;
    }
    c->rlen=0;
    c->wpos=0;
    c->wlen=0;
    aeDeleteFileEvent(server.el,c->fd,AE_WRITABLE);
    aeDeleteFileEvent(server.el,c->fd,AE_READABLE);
    aeCreateFileEvent(server.el, c->fd, AE_READABLE,
        readQueryFromClient, c);
}

static void daemonize(void) {
    int fd;
    FILE *fp;

    if (fork() != 0) exit(0); /* parent exits */
    setsid(); /* create a new session */

    /* Every output goes to /dev/null. If Redis is daemonized but
     * the 'logfile' is set to 'stdout' in the configuration file
     * it will not log at all. */
    if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
        dup2(fd, STDIN_FILENO);
        dup2(fd, STDOUT_FILENO);
        dup2(fd, STDERR_FILENO);
        if (fd > STDERR_FILENO) close(fd);
    }
    /* Try to write the pid file */
    fp = fopen(server.pidfile,"w");
    if (fp) {
        fprintf(fp,"%d\n",getpid());
        fclose(fp);
    }
}


static void initServerConfig() {
    server.port = REDIS_SERVERPORT;
    server.verbosity = REDIS_FATAL ;
    server.maxidletime = REDIS_MAXIDLETIME;
    server.logfile = NULL; /* NULL = log on standard output */
    server.bindaddr = NULL;
    server.daemonize = 0;
    server.pidfile = "/var/run/redis.pid";
    server.maxclients = 1024;
}

static void initServer() {
    signal(SIGHUP, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    setupSigSegvAction();

    server.mainthread = pthread_self();
    server.devnull = fopen("/dev/null","w");
    if (server.devnull == NULL) {
        redisLog(REDIS_WARNING, "Can't open /dev/null: %s", server.neterr);
        exit(1);
    }
    server.clients = 0;
    server.el = aeCreateEventLoop();
    server.fd = anetTcpServer(server.neterr, server.port, server.bindaddr);
    if (server.fd == -1) {
        redisLog(REDIS_WARNING, "Opening TCP port: %s", server.neterr);
        exit(1);
    }
    server.stat_numcommands = 0;
    server.stat_numconnections = 0;
    server.stat_starttime = time(NULL);
    server.unixtime = time(NULL);
    //aeCreateTimeEvent(server.el, 1, serverCron, NULL, NULL);
    if (aeCreateFileEvent(server.el, server.fd, AE_READABLE,
        acceptHandler, NULL) == AE_ERR) oom("creating file event");

}

static void version() {
    printf("Redis server version %s\n", REDIS_VERSION);
    return;
}

static void usage(void) {
    version();
    printf("-p <num>      TCP port number to listen on (default: 6376)\n"
           "-l <ip_addr>  interface to listen on (default: INADDR_ANY, all addresses)\n"
           "-d            run as a daemon\n"
           "-c <num>      max simultaneous connections (default: 1024)\n"
           "-v            verbose (print errors/warnings while in event loop)\n"
           "-u <num>      the unique server id, must be a number between 1 and 255\n"
           );
    return;
}

int main(int argc, char **argv) {
    REDIS_NOTUSED(argc);
    REDIS_NOTUSED(argv);
    initServerConfig();
    mid=0;
    sid=0;
    int c;
    /* process arguments */
    while (-1 != (c = getopt(argc, argv,
          "p:"  /* TCP port number to listen on */
          "c:"  /* max simultaneous connections */
          "u:"  /* unique server id */
          "v"   /* verbose */
          "d"   /* daemon mode */
          "h"   /* show usage */
          "l:"  /* interface to listen on */
        ))) {
        switch (c) {
        case 'p':
            server.port= atoi(optarg);
            break;
        case 'c':
            server.maxclients= atoi(optarg);
            break;
        case 'u':
            mid= atoi(optarg);
            if (mid < 1 || mid > 255) {
                fprintf(stderr, "mid must be a number between 1 and 255.\n");
                return 1;
            }
            break;
        case 'v':
            server.verbosity = REDIS_DEBUG;
            break;
        case 'd':
            server.daemonize = 1;
            break;
        case 'h':
            usage();
            return 1;
            break;
        case 'l':
            server.bindaddr = strdup(optarg);
            break;  
        default:
            fprintf(stderr, "Illegal argument \"%c\"\n", c);
            usage();
            return 1;
        }
    }
    if (mid < 1 || mid > 255) {
        fprintf(stderr, "mid must be a number between 1 and 255.\n");
        return 1;
    }
    if (server.daemonize) daemonize();
    redisLog(REDIS_NOTICE,"Server started, Redis version " REDIS_VERSION);
    initServer();
    redisLog(REDIS_NOTICE,"The server is now ready to accept connections on port %d", server.port);
    aeMain(server.el);
    aeDeleteEventLoop(server.el);
    return 0;
}
