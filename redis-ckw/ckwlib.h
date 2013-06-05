#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <assert.h>

#define MAX_NODE 2*1024*1024 
#define KW_MAX_LEN 128

typedef struct _cell {
	unsigned int point:31;
	unsigned int end:1;
}cell;

typedef struct _node {
	 cell cell[256];
}node;

void *create_db(char *name,int num);
void *open_db(char *name,int *num);
void close_db(void *m,int size);
int set_nodes(node *nodes,int *used);
int get_nodes(node *nodes,char *s);
int check_keywords(node *nodes,char *s);
