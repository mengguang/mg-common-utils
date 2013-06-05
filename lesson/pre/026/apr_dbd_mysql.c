#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_strings.h>
#include <apr_dbd.h>

void apr_err(const char *s, apr_status_t rv)
{
    char buf[120];
    fprintf(stderr, "%s: %s (%d)\n", s, apr_strerror(rv, buf, sizeof buf), rv);
}

//fprintf(stderr, "%s: %s (%d)\n", s, apr_dbd_error(driver,handle,rv));


int main(int argc,char **argv) {
    apr_initialize();
    apr_pool_t *pool;
    apr_pool_create(&pool,NULL);
    apr_status_t st;
    
    apr_dbd_t *db;
    apr_dbd_init(pool);
    const apr_dbd_driver_t *driver;
    st=apr_dbd_get_driver(pool,"mysql",&driver);
    if(st != APR_SUCCESS) {
        apr_err("apr_dbd_get_driver()",st);
        return st;
    }
    char *params=apr_psprintf(pool,"host=%s,port=%d,user=%s,pass=%s,dbname=%s","10.1.1.161",3306,"mengguang","1234qwer","test");
    st=apr_dbd_open(driver,pool,params,&db);
    if(st != APR_SUCCESS) {
        apr_err("apr_dbd_open()",st);
        return st;
    }
    char *fmt="insert into friends (name,email) values ('%s','%s')";
    char *sql=apr_psprintf(pool,fmt,"laomeng","laomeng188@163.com");
    int nrows=0;
    st=apr_dbd_query(driver,db,&nrows,sql);
    printf("nrows = %d\n",nrows);
    char *sel="select name,email from friends";
    apr_dbd_results_t *res=NULL;
    st=apr_dbd_select(driver,pool,db,&res,sel,-1);
    if(st != APR_SUCCESS) {
        apr_err("apr_dbd_select()",st);
        fprintf(stderr, "%s\n", apr_dbd_error(driver,db,st));
        return st;
    }
    nrows=apr_dbd_num_tuples(driver,res);
    printf("rows of result: %d\n",nrows);
    int ncols=apr_dbd_num_cols(driver,res);
    printf("cols of result: %d\n",ncols);
    apr_dbd_row_t *row;
    for(int i=1; -1 != apr_dbd_get_row(driver,pool,res,&row,i);i++ ) {
        printf("name = %s, email = %s\n",apr_dbd_get_entry(driver,row,0),apr_dbd_get_entry(driver,row,1)); 
    } 

    apr_dbd_close(driver,db);

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

