/*

UDP protocol
------------

For very large installations where the number of clients is high enough
that the number of TCP connections causes scaling difficulties, there is
also a UDP-based interface. The UDP interface does not provide guaranteed
delivery, so should only be used for operations that aren't required to
succeed; typically it is used for "get" requests where a missing or
incomplete response can simply be treated as a cache miss.

Each UDP datagram contains a simple frame header, followed by data in the
same format as the TCP protocol described above. In the current
implementation, requests must be contained in a single UDP datagram, but
responses may span several datagrams. (The only common requests that would
span multiple datagrams are huge multi-key "get" requests and "set"
requests, both of which are more suitable to TCP transport for reliability
reasons anyway.)

The frame header is 8 bytes long, as follows (all values are 16-bit integers
in network byte order, high byte first):

0-1 Request ID
2-3 Sequence number
4-5 Total number of datagrams in this message
6-7 Reserved for future use; must be 0

The request ID is supplied by the client. Typically it will be a
monotonically increasing value starting from a random seed, but the client
is free to use whatever request IDs it likes. The server's response will
contain the same ID as the incoming request. The client uses the request ID
to differentiate between responses to outstanding requests if there are
several pending from the same server; any datagrams with an unknown request
ID are probably delayed responses to an earlier request and should be
discarded.

The sequence number ranges from 0 to n-1, where n is the total number of
datagrams in the message. The client should concatenate the payloads of the
datagrams for a given response in sequence number order; the resulting byte
stream will contain a complete response in the same format as the TCP
protocol (including terminating \r\n sequences).

*/

#include "redis.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int anetCreateUdpSocket(char *err);
int headerEncode(char *wbuf,int req,int seq,int ngram);
int headerDecode(unsigned char *hdr,int *req,int *seq,int *ngram);
int getCommandforUdp(char *key,char *value,int vlen);
int anetUdpServer(char *err, int port, char *bindaddr);
int anetUdpRecvFrom(char *err, int s, struct sockaddr *addr, void *buf, int buf_len);
int anetUdpSendTo(char *err, int s, struct sockaddr *addr, void *buf, int len);
void processCommandFromUdpClient(aeEventLoop *el, int fd, void *privdata, int mask);
