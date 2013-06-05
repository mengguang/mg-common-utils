#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <my_global.h>
#include <my_sys.h>
#include <m_string.h>
#include <mysql.h>
#include <mysql_embed.h>
#include <errmsg.h>
#include <my_getopt.h>
#include "sql_common.h"
#include "log_event_mini.h"


int main(int argc,char **argv)
{
	MYSQL *m=mysql_init(NULL);
	if(!m) return -1;
	char *host="10.210.74.152";
	char *user="mg";
	char *pass="123qwe";
	char *db="test";
	unsigned short port=3306;
	if(!mysql_real_connect(m,host,user,pass,db,port,NULL,0))
	{
		printf("%s\n",mysql_error(m));
		return -2;
	}
	unsigned char buf[128];
	unsigned char *bufp=buf;
        memset(buf,0,sizeof(buf));
        int4store(buf,(uint32)4);	//start position
        int2store(buf+4,(uint32)0);	//flags
        char *logname="mysql-bin.000001";
        size_t len=strlen(logname);
        int4store(buf+6,(uint32)0);	//slave id
        memcpy(buf+10,logname,len);	//logname
        if(simple_command(m,COM_BINLOG_DUMP,buf,len+10,1))
	{
                return -3;
        	printf("%s\n",mysql_error(m));
	}
	FILE *fp=fopen("mysql-bin.000001","w");
	fwrite(BINLOG_MAGIC,4,1,fp);
	if(!fp){
		perror("fopen");
		return -4;
	}
	char *rbuf;
	int begin=1;
	for(;;)
	{
		size_t len=cli_safe_read(m);
		if(len == packet_error || len < 1){
			printf("packet error\n");
			break;
		}
		if(len < 8 && m->net.read_pos[0] == 254){
			 break;
		}
		rbuf=m->net.read_pos+1;
		
		if(rbuf[EVENT_TYPE_OFFSET] == ROTATE_EVENT){
			if(begin == 1){
				begin=0;
				continue;
			} else {
				begin=1;
				fwrite(rbuf,len-1,1,fp);
				break;
			}
		}
		fwrite(rbuf,len-1,1,fp);
	}
	fclose(fp);
	mysql_close(m);
	return 0;
}
