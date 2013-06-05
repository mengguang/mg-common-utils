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
    st=apr_dbm_open(&dbm,"friends",APR_DBM_RWCREATE,APR_OS_DEFAULT ,pool);

    apr_datum_t key,value;
    key.dptr="laomeng";
    key.dsize=strlen(key.dptr);
    
    value.dptr="laomeng188@163.com";
    value.dsize=strlen(value.dptr);
   
    st=apr_dbm_store(dbm,key,value);

    apr_datum_t pvalue;
    st=apr_dbm_fetch(dbm,key,&pvalue);

    char *pstr=apr_pstrndup(pool,pvalue.dptr,pvalue.dsize);
    printf("value => %s\n",pstr);
    
    apr_dbm_close(dbm);

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

