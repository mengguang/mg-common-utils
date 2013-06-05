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
#include "adlist.h"
#include "redis.h"

#define REDIS_VERSION "2.0.4"

/* Error codes */
#define REDIS_OK                0
#define REDIS_ERR               -1

/* Static server configuration */
#define REDIS_SERVERPORT        6378    /* TCP port */
#define REDIS_MAXIDLETIME       (60*5)  /* default client timeout */

/* List related stuff */
#define REDIS_HEAD 0
#define REDIS_TAIL 1

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
    list *io_newjobs;
    list *io_processed;
    char neterr[ANET_ERR_LEN];
    aeEventLoop *el;
    int cronloops;              /* number of times the cron function run */
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
    unsigned int io_active_threads;
    unsigned int vm_max_threads;
    time_t unixtime;    /* Unix time sampled every second. */
    int shutdown_asap;
    pthread_mutex_t io_mutex; /* lock to access io_jobs/io_done/io_thread_job */
    pthread_cond_t io_ready;
    pthread_attr_t io_threads_attr; /* attributes for threads creation */
    int io_ready_pipe_write;
    int io_ready_pipe_read;
    char *extension;
    FILE *devnull;
};
typedef struct RedisServer redisServer;

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
static void *IOThreadEntryPoint(void *arg);
static void spawnIOThread(void);
static void queueIOJob(redisClient *c);
static void sendReplyToClient(aeEventLoop *el, int fd, void *privdata, int mask);
static void beforeSleep(struct aeEventLoop *eventLoop);
//static int serverCron(struct aeEventLoop *eventLoop, long long id, void *clientData);
static void sigtermHandler(int sig);
static void segvHandler(int sig, siginfo_t *info, void *secret);
static void freeClient(redisClient *c);
static void resetClient(redisClient *c);
//static void processCommand(redisClient *c,vod *privptr);
static void vmThreadedIOCompletedJob(aeEventLoop *el, int fd, void *privdata,int mask);
static void noticeThreadedIO(void);
static void waitThreadedIO(void);
static void lockThreadedIO(void);
static void unlockThreadedIO(void);
static void oom(const char *msg);

static redisServer server;

redisCommandInit *extCmdInit;
redisCommandProc *extCmdProc;
redisCommandDeinit *extCmdDeinit;
processRequestBuffer  *extProcessRequestBuffer;

int loadExtension(char *extName) {
    void *ext=dlopen(extName,RTLD_LAZY);
    if(!ext) {
        redisLog(REDIS_FATAL,"Load extension %s error: %s",extName,dlerror());
        return REDIS_ERR;
    }
    extCmdInit=(redisCommandInit *)dlsym(ext,"_redisCommandInit");
    if(!extCmdInit) {
        redisLog(REDIS_FATAL,"Load symbol _redisCommandInit error: %s",dlerror());
        return REDIS_ERR;
    }
    extCmdProc=(redisCommandProc *)dlsym(ext,"_redisCommandProc");
    if(!extCmdProc) {
        redisLog(REDIS_FATAL,"Load symbol _redisCommandProc error: %s",dlerror());
        return REDIS_ERR;
    }
    extCmdDeinit=(redisCommandDeinit *)dlsym(ext,"_redisCommandDeinit");
    if(!extCmdDeinit) {
        redisLog(REDIS_FATAL,"Load symbol _redisCommandDeinit error: %s",dlerror());
        return REDIS_ERR;
    }
    extProcessRequestBuffer=(processRequestBuffer *)dlsym(ext,"_processRequestBuffer");
    if(!extProcessRequestBuffer) {
        redisLog(REDIS_FATAL,"Load symbol _processRequestBuffer error: %s",dlerror());
        return REDIS_ERR;
    }
    return REDIS_OK;
}

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

/*
static int serverCron(struct aeEventLoop *eventLoop, long long id, void *clientData) {
    server.cronloops++;
    REDIS_NOTUSED(eventLoop);
    REDIS_NOTUSED(id);
    REDIS_NOTUSED(clientData);
    server.unixtime = time(NULL);
    return 100;
}
*/
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
        c->lastinteraction = time(NULL);
    } else {
        return;
    }
    processInputBuffer(c);
}



static void processInputBuffer(redisClient *c) {
    int v=(*extProcessRequestBuffer)(c->rbuf,c->rlen);
    if(v == -1) {
        if(server.verbosity <= REDIS_VERBOSE )
            redisLog(REDIS_VERBOSE, "Client Protocol Error");
        freeClient(c);
        return;
    }
    if(v == -2) { //QUIT command
        freeClient(c);
    }
    if(v == 0) {
        return;
    }
    if(v == 1) {
        aeDeleteFileEvent(server.el,c->fd,AE_READABLE);
        queueIOJob(c);
    }
}

static void initServerConfig() {
    server.port = REDIS_SERVERPORT;
    server.verbosity = REDIS_FATAL;
    server.maxidletime = REDIS_MAXIDLETIME;
    server.logfile = NULL; /* NULL = log on standard output */
    server.bindaddr = NULL;
    server.daemonize = 1;
    server.pidfile = "/var/run/redis.pid";
    server.maxclients = 0;
    server.shutdown_asap = 0;
    server.vm_max_threads = 100;
    server.extension="./echo.so";
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
    if(loadExtension(server.extension) == REDIS_ERR) {
        exit(1);
    }
    server.clients = 0;
    server.el = aeCreateEventLoop();
    server.fd = anetTcpServer(server.neterr, server.port, server.bindaddr);
    if (server.fd == -1) {
        redisLog(REDIS_WARNING, "Opening TCP port: %s", server.neterr);
        exit(1);
    }
    server.cronloops = 0;
    server.stat_numcommands = 0;
    server.stat_numconnections = 0;
    server.stat_starttime = time(NULL);
    server.unixtime = time(NULL);
    //aeCreateTimeEvent(server.el, 1, serverCron, NULL, NULL);
    if (aeCreateFileEvent(server.el, server.fd, AE_READABLE,
        acceptHandler, NULL) == AE_ERR) oom("creating file event");

    /* Initialize threaded I/O (used by Virtual Memory) */
    server.io_newjobs = listCreate();
    server.io_processed = listCreate();
    pthread_mutex_init(&server.io_mutex,NULL);
    pthread_cond_init(&server.io_ready,NULL);
    server.io_active_threads = 0;
    int pipefds[2];
    if (pipe(pipefds) == -1) {
        redisLog(REDIS_WARNING,"Unable to intialized VM: pipe(2): %s. Exiting."
            ,strerror(errno));
        exit(1);
    }
    server.io_ready_pipe_read = pipefds[0];
    server.io_ready_pipe_write = pipefds[1];
    redisAssert(anetNonBlock(NULL,server.io_ready_pipe_read) != ANET_ERR);
    int i;
    for(i=0;i<server.vm_max_threads;i++) {
        spawnIOThread();
    }
    /* Listen for events in the threaded I/O pipe */
    if (aeCreateFileEvent(server.el, server.io_ready_pipe_read, AE_READABLE,
        vmThreadedIOCompletedJob, NULL) == AE_ERR)
        oom("creating file event");
}


/* Every time a thread finished a Job, it writes a byte into the write side
 * of an unix pipe in order to "awake" the main thread, and this function
 * is called.
 */
static void vmThreadedIOCompletedJob(aeEventLoop *el, int fd, void *privdata, int mask) {
    char buf[1];
    listNode *ln;
    int retval;
    REDIS_NOTUSED(el);
    REDIS_NOTUSED(mask);
    REDIS_NOTUSED(privdata);

    /* For every byte we read in the read side of the pipe, there is one
     * I/O job completed to process. */
    while((retval = read(fd,buf,1)) == 1) {
        redisClient *c;
        if(server.verbosity <= REDIS_DEBUG )
            redisLog(REDIS_DEBUG,"Processing I/O completed job");

        /* Get the processed element (the oldest one) */
        lockThreadedIO();
        assert(listLength(server.io_processed) != 0);
        ln = listFirst(server.io_processed);
        c = ln->value;
        listDelNode(server.io_processed,ln);
        unlockThreadedIO();

        /* Post process it in the main thread, as there are things we
         * can do just here to avoid race conditions and/or invasive locks */
        if(server.verbosity <= REDIS_DEBUG )
            redisLog(REDIS_DEBUG,"Main thread got a job from processed queue.");
        if (aeCreateFileEvent(server.el, c->fd, AE_WRITABLE,
            sendReplyToClient, c) == AE_ERR) return;
    }
    if (retval < 0 && errno != EAGAIN) {
        redisLog(REDIS_WARNING,
            "WARNING: read(2) error in vmThreadedIOCompletedJob() %s",
            strerror(errno));
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
    server.shutdown_asap = 1;
}

static void segvHandler(int sig, siginfo_t *info, void *secret) {
    REDIS_NOTUSED(info);
    REDIS_NOTUSED(secret);
    redisLog(REDIS_WARNING,
        "======= Ooops! Redis %s got signal: -%d- =======", REDIS_VERSION, sig);
    _exit(0);
}

/* This function gets called every time Redis is entering the
 * main loop of the event driven library, that is, before to sleep
 * for ready file descriptors. */
static void beforeSleep(struct aeEventLoop *eventLoop) {
    REDIS_NOTUSED(eventLoop);
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

/* This function must be called while with threaded IO locked */
static void queueIOJob(redisClient *c) {
    if(server.verbosity <= REDIS_DEBUG )
        redisLog(REDIS_DEBUG,"Main thread put an IO Job to queue.");
    lockThreadedIO();
    listAddNodeTail(server.io_newjobs,c);
    unlockThreadedIO();
    noticeThreadedIO();
    //if (server.io_active_threads < server.vm_max_threads)
    //    spawnIOThread();
}

static void spawnIOThread(void) {
    pthread_t thread;
    sigset_t mask, omask;
    int err;

    sigemptyset(&mask);
    sigaddset(&mask,SIGCHLD);
    sigaddset(&mask,SIGHUP);
    sigaddset(&mask,SIGPIPE);
    pthread_sigmask(SIG_SETMASK, &mask, &omask);
    while ((err = pthread_create(&thread,NULL,IOThreadEntryPoint,NULL)) != 0) {
        redisLog(REDIS_WARNING,"Unable to spawn an I/O thread: %s",
            strerror(err));
        usleep(1000000);
    }
    pthread_sigmask(SIG_SETMASK, &omask, NULL);
    server.io_active_threads++;
}

static void noticeThreadedIO(void) {
    pthread_cond_signal(&server.io_ready);
}

static void waitThreadedIO(void) {
    pthread_cond_wait(&server.io_ready,&server.io_mutex);
}

static void lockThreadedIO(void) {
    pthread_mutex_lock(&server.io_mutex);
}

static void unlockThreadedIO(void) {
    pthread_mutex_unlock(&server.io_mutex);
}

static void *IOThreadEntryPoint(void *arg) {
    redisClient *c;
    listNode *ln;
    REDIS_NOTUSED(arg);
    void *privptr=NULL;
    (*extCmdInit)(&privptr);
    pthread_detach(pthread_self());
    while(1) {
        /* Get a new job to process */
        lockThreadedIO();
        while (listLength(server.io_newjobs) == 0) {
            /* No new jobs in queue, exit. */
            if(server.verbosity <= REDIS_DEBUG )
                redisLog(REDIS_DEBUG,"Thread %ld, nothing to do, just wait...",
                    (long) pthread_self());
            waitThreadedIO();
        }
        ln = listFirst(server.io_newjobs);
        c = ln->value;
        listDelNode(server.io_newjobs,ln);
        unlockThreadedIO();
        if(server.verbosity <= REDIS_DEBUG )
            redisLog(REDIS_DEBUG,"Thread %ld got a new job to process",(long) pthread_self());

        /* Process the Job */
	    (*extCmdProc)(c,&privptr);
	
        /* Done: insert the job into the processed queue */
        if(server.verbosity <= REDIS_DEBUG )
            redisLog(REDIS_DEBUG,"Thread %ld completed the job",(long) pthread_self());
        lockThreadedIO();
        listAddNodeTail(server.io_processed,c);
        unlockThreadedIO();

        /* Signal the main thread there is new stuff to process */
        assert(write(server.io_ready_pipe_write,"x",1) == 1);
    }
    return NULL; /* never reached */
}
/*
static void processCommand(redisClient *c,void *privptr) {
    redisLog(REDIS_DEBUG,"processCommand");
    (*extCmdProc)(c,privptr);
}
*/
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

static void version() {
    printf("Redis server version %s\n", REDIS_VERSION);
    exit(0);
}

static void usage() {
    fprintf(stderr,"Usage: mgcsf\n");
    version();
    exit(1);
}

int main(int argc, char **argv) {
    REDIS_NOTUSED(argc);
    REDIS_NOTUSED(argv);
    if(argc != 1)
    	usage();
    time_t start;
    start = time(NULL);
    initServerConfig();
    if (server.daemonize) daemonize();
    redisLog(REDIS_NOTICE,"Server started, Redis version " REDIS_VERSION);
    initServer();
    redisLog(REDIS_NOTICE,"The server is now ready to accept connections on port %d", server.port);
    aeSetBeforeSleepProc(server.el,beforeSleep);
    aeMain(server.el);
    aeDeleteEventLoop(server.el);
    return 0;
}
