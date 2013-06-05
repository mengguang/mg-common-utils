#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_strings.h>
#include <apr_user.h>

void apr_err(const char *s, apr_status_t rv)
{
    char buf[120];
    fprintf(stderr, "%s: %s (%d)\n", s, apr_strerror(rv, buf, sizeof buf), rv);
}

int main(int argc,char **argv) {
    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool,NULL);
    apr_status_t st;

    apr_uid_t uid;
    apr_gid_t gid;
    
    apr_uid_current(&uid,&gid,pool);
    char *username;
    apr_uid_name_get(&username,uid,pool);
    printf("username: %s\n",username);
    
    char *dirname;
    apr_uid_homepath_get(&dirname,username,pool);
    printf("homepath: %s\n",dirname);
    
    char *group;
    apr_gid_name_get(&group,gid,pool);
    printf("group: %s\n",group);

    apr_uid_t myuid;
    apr_uid_t mygid;
    apr_uid_get(&myuid,&mygid,username,pool);
    if(apr_uid_compare(myuid,uid) == APR_SUCCESS) {
        printf("same uid\n");
    }
    if(apr_gid_compare(mygid,gid) == APR_SUCCESS) {
        printf("same gid\n");
    }

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

