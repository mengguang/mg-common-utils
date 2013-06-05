#include <iostream>
#include <queue>
#include <list>
#include <mysql/mysql.h>
#include <pthread.h>

#include <concurrency/ThreadManager.h>
#include <concurrency/PosixThreadFactory.h>
#include <protocol/TBinaryProtocol.h>
#include <server/TSimpleServer.h>
#include <server/TThreadPoolServer.h>
#include <server/TThreadedServer.h>
#include <transport/TServerSocket.h>
#include <transport/TTransportUtils.h>

#include "HandlerService.h"

using namespace std;

char *mysql_unix_socket="/var/lib/mysql/mysql.sock";
char *mysql_user="root";
char *mysql_password=NULL;
char *mysql_database="test";
char *handler_stmt_open="handler kv open";
char *handler_stmt_close="handler kv close";
char *handler_stmt_read="handler kv read `PRIMARY`=(%u)";

queue<MYSQL *> mysql_handler_queue;
pthread_mutex_t handler_queue_lock=PTHREAD_MUTEX_INITIALIZER;


bool close_mysql_handler(MYSQL *mysql) {
	mysql_close(mysql);
	free(mysql);
}

MYSQL *new_mysql_handler() {
	MYSQL *mysql=(MYSQL *)calloc(sizeof(MYSQL),1);
	mysql_init(mysql);
	if(!mysql_real_connect(mysql,NULL,mysql_user,mysql_password,mysql_database,0,mysql_unix_socket,0)) {
		fprintf(stderr,"Failed to connect to database: Error : %s\n",mysql_error(mysql));
		return NULL;
	}
	if(mysql_query(mysql,handler_stmt_open)) {
		fprintf(stderr,"Failed to open handler: Error : %s\n",mysql_error(mysql));
		close_mysql_handler(mysql);
		return NULL;
	}
	return mysql;
}

bool put_mysql_handler(MYSQL *mysql) {
	pthread_mutex_lock(&handler_queue_lock);
	mysql_handler_queue.push(mysql);
	pthread_mutex_unlock(&handler_queue_lock);
	return true;
}


MYSQL *get_mysql_handler() {
	pthread_mutex_lock(&handler_queue_lock);
	if(mysql_handler_queue.size() == 0) {
		pthread_mutex_unlock(&handler_queue_lock);
		return new_mysql_handler();
	} else {
		MYSQL *mysql=mysql_handler_queue.front();
		mysql_handler_queue.pop();
		pthread_mutex_unlock(&handler_queue_lock);
		return mysql;
	}
}


bool handler_read(unsigned int key,string &value) {
	MYSQL *mysql=get_mysql_handler();
	if(mysql == NULL) {
		fprintf(stderr,"Failed to get mysql handler.");
		return false;
	}
	char stmt_buf[1024]={0};
	sprintf(stmt_buf,handler_stmt_read,key);
	if(mysql_query(mysql,stmt_buf)) {
		fprintf(stderr,"Failed to read handler: Error : %s\n",mysql_error(mysql));
		close_mysql_handler(mysql);
		return false;
	}
	MYSQL_RES *result;
	result=mysql_store_result(mysql);
	put_mysql_handler(mysql);
	if(result) {
		MYSQL_ROW row;
		row=mysql_fetch_row(result);
		if(row) {
			unsigned long *lengths;
			lengths=mysql_fetch_lengths(result);
			value.assign(row[1],lengths[1]);
			mysql_free_result(result);
			return true;
		} else {
			mysql_free_result(result);
			return false;
		}
	}
}


int __main(int argc,char **argv) {
	/*
	for(unsigned int i=0;i<10;i++) {
		string value;
		if(handler_read(i,value)) {
			cout << "value for key: " << i << " => " << value << endl;
		} else {
			cout << "value for key: " << i << " not found." << endl;
		}
	}
	*/
	int count=10000;
	if(argc == 2) {
		count=atoi(argv[1]);
	}
	string value;
	for(;count > 0;count--) {
		handler_read(6,value);
		if(count % 1000 == 0) {
			cout << count << endl;
		}
	}
}

