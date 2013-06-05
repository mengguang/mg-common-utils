#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_dbm.h>
#include <apr_strings.h>

int main(int argc,char **argv) {
    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool,NULL);
    apr_status_t st;

    apr_dbm_t *dbm;
    st=apr_dbm_open(&dbm,"friends",APR_DBM_RWCREATE,APR_OS_DEFAULT,pool);
    if(st != APR_SUCCESS) {
        fprintf(stderr,"apr_dbm_open : %d\n",st);
        return st;
    }

    apr_datum_t key,value;
    key.dptr="laomeng";
    key.dsize=strlen(key.dptr)+1;
    value.dptr="laomeng188@163.com";
    value.dsize=strlen(value.dptr)+1;

    st=apr_dbm_store(dbm,key,value);
    
    apr_datum_t pvalue;
    st=apr_dbm_fetch(dbm,key,&pvalue);
    
    printf("pvalue => %s\n",pvalue.dptr);
    printf(" value => %s\n",value.dptr);

    apr_dbm_close(dbm);
    



    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}
