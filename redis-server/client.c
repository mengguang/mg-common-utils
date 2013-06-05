#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "anet.h"

int main(int argc,char **argv)
{
	char buf[14];
	char buf2[14];
	int *size=(int *)buf;
	*size=14;
	strncpy(buf+4,"111111111",10);
	char *addr="127.0.0.1";
	int port=6379;
	int fd;
	fd=anetTcpConnect(NULL,addr,port);
	if(fd == ANET_ERR) return -1;
	int i=0;
	for(i=0;i<1000000;i++)
	{
		anetWrite(fd,buf,14);
		memset(buf2,0,14);
		anetRead(fd,buf2,14);
		if(i % 1000 == 0) printf("%d\n",i);
		//puts(buf+4);
	}
}
