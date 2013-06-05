#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <math.h>
#include <bitset>
#include <climits>

using namespace std;

long ip2long(char *ip) {
	long lip=0;
	if(!ip) {
		return -1;
	}
	char *p1=strchr(ip,'.');
	if(!p1) {
		return -1;
	}
	char *p2=strchr(p1+1,'.');
	if(!p2) {
		return -2;
	}
	char *p3=strchr(p2+1,'.');
	if(!p3) {
		return -3;
	}
	lip+=atol(strndupa(ip,p1-ip)) << 24;
	lip+=atol(strndupa(p1+1,p2-(p1+1))) << 16;
	lip+=atol(strndupa(p2+1,p3-(p2+1))) << 8;
	lip+=atol(p3+1);
	return lip;
}

char *long2ip(long ip) {
	char *sip=(char *)calloc(64,1);
	unsigned uiip=(unsigned int)ip;
	sprintf(sip,"%u.%u.%u.%u",(( uiip << 0) >> 24),((uiip << 8) >> 24),((uiip << 16) >> 24),((uiip << 24) >> 24));
	return sip;
}
/*
    $corr=(pow(2,32)-1)-(pow(2,32-$mask)-1);
    $first=ip2long($ip) & ($corr);
    $length=pow(2,32-$mask)-1;
*/
long get_first_ip(char *cidr, char *mask, long *length) {
	long lmask=atol(mask);
	long ip_mask= ~((1 << (32 - lmask)) -1);
	long first=(ip2long(cidr) & ip_mask) + 1;
	if(length) {
		*length=pow(2,32-lmask)-1;
	}
	return first;
}

int main(int argc,char **argv) {
	if(argc != 2) {
		printf("usage: %s cidr_files\n",argv[0]);
		return -1;
	}
	FILE *fp=fopen(argv[1],"r");
	if(!fp) {
		perror("fopen:");
		return -2;
	}
	
	bitset<UINT_MAX > *bs= new bitset<UINT_MAX>;
	//bitset<1 > *bs= new bitset<1>;
	char cidr[64]={0};
	int nc=0;
	char ip[64]={0};
	char mask[64]={0};
	long j=0;
	while(!feof(fp)) {
		//break;
		memset(cidr,0,sizeof(cidr));
		memset(ip,0,sizeof(ip));
		memset(mask,0,sizeof(mask));
		fgets(cidr,63,fp);
		nc=strlen(cidr);
		if(nc <= 7) {
			if( nc > 1) {
				printf("bad cidr: %s\n",cidr);
			}
			continue;
		}
		cidr[nc-1]='\0';
		char *slash=strchr(cidr,'/');
		if(!slash) {
			printf("bad cidr: %s\n",cidr);
			continue;
		}
		strncpy(ip,cidr,slash-cidr);
		strncpy(mask,slash+1,63);
		long length=0;
		long first=get_first_ip(ip,mask,&length);
		for(long i=first;i<(first+length);i++) {
			bs->set(i);
			j++;
		}
	}
	fclose(fp);	

	int c=0;
	char cip[64];
	int total=0;
	while(!feof(stdin)) {
		fscanf(stdin," %d %s\n",&c,cip);
		if(ip2long(cip) >= UINT_MAX || ip2long(cip) <=0 ) {
			printf("invalied ip: %s\n",cip);
			continue;
		}
		if(bs->test(ip2long(cip))) {
			printf("%d %s\n",c,cip);
			total+=c;
		}
	}
	printf("total count : %d\n",total);

	return 0;
}

