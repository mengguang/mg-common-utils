#include "ckwlib.h"

int set_nodes(node *nodes,int *used,char *s) {
	int p=0,rp=0;
	while( *s != '\0' ) {
		rp=p;
		if(nodes[p].cell[(unsigned char)*s].point == 0 && *(s+1) != '\0' ) {
			if(*used + 1 > MAX_NODE) {
			    return -1;
			}
			*used++;
			nodes[p].cell[(unsigned char)*s].point=*used;
			p=*used;
		} else {
			p=nodes[p].cell[(unsigned char)*s].point;
		}
		s++;
	}
	nodes[rp].cell[(unsigned char)*(s-1)].end=1;
	return 0;
}

int get_nodes(node *nodes,char *s) {
	int p=0;
	while( *s != '\0' ) {
		if(nodes[p].cell[(unsigned char)*s].end == 1) {
			return 1;
		} else {
			p=nodes[p].cell[(unsigned char)*s].point;
		}
		s++;
	}
	return 0;
}

int check_keywords(node *nodes,char *s) {
	int cr=0;
	char *o=s;
	while ( *s != '\0' ) {
		cr=get_nodes(nodes,s,k);
		if(cr == 1) {
		    return s-o;
		} else {
			s++;
		}
	}
	return -1;
}

void *open_db(char *name,int *size) {
    int fd,flag;
    void *m;
    flag=O_RDWR;
    if((fd=open(name,flag,S_IRWXU)) <= 0) {
        return NULL;
    }
    struct stat st;
    if(fstat(fd,&st) == -1 ) {
        close(fd);
        return NULL;
    }
    *size=st.st_size;
    if((m=mmap(0,st.st_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0)) == MAP_FAILED) {
        return NULL;
    }
    close(fd);
    return m;
}

void *create_db(char *name,int size) {
    int fd,flag;
    void *m;
    flag=O_RDWR|O_CREAT|O_TRUNC;
    if((fd=open(name,flag,S_IRWXU)) <= 0) {
        return NULL;
    }
    if(ftruncate(fd,size) != 0) {
        return NULL;
    }
    if((m=mmap(0,size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0)) == MAP_FAILED) {
        return NULL;
    }
    memset(m,0,size);
    close(fd);
    return m;
}

void close_db(void *m,int size) {
	if(m != NULL)
		munmap(m,size);
	return;
}
