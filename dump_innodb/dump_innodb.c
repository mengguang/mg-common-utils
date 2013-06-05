#include <stdio.h>
#include <stdlib.h>
#include <haildb.h>
#include <test0aux.h>
#include <assert.h>

int main(int argc,char **argv){
    if(argc != 2){
        printf("give me a database\table name\n");
        return 1;
    }
    char *dtname=argv[1];
    
    ib_err_t err;
    err=ib_init();
    if(err != DB_SUCCESS){
        puts(ib_strerror(err));
        return err;
    }
    
    err = ib_cfg_set_int("log_buffer_size", 8*1024*1024);
    if(err != DB_SUCCESS){
        puts(ib_strerror(err));
        return err;
    }
    err = ib_cfg_set_int("force_recovery", 1);
    if(err != DB_SUCCESS){
        puts(ib_strerror(err));
        return err;
    }
    err = ib_cfg_set_int("log_file_size", 128*1024*1024);
    if(err != DB_SUCCESS){
        puts(ib_strerror(err));
        return err;
    }
    err = ib_cfg_set_int("log_files_in_group", 3);
    if(err != DB_SUCCESS){
        puts(ib_strerror(err));
        return err;
    }
    err = ib_cfg_set_text("log_group_home_dir", "./");
    //err = ib_cfg_set_text("log_group_home_dir", "/var/lib/mysql/");
    if(err != DB_SUCCESS){
        puts(ib_strerror(err));
        return err;
    }
    err = ib_cfg_set_text("data_home_dir", "./");
    if(err != DB_SUCCESS){
        puts(ib_strerror(err));
        return err;
    }
    err = ib_cfg_set_text("data_file_path", "ibdata1:500M:autoextend");
    if(err != DB_SUCCESS){
        puts(ib_strerror(err));
        return err;
    }
    err = ib_cfg_set_bool_on("file_per_table");
    if(err != DB_SUCCESS){
        puts(ib_strerror(err));
        return err;
    }

    err=ib_startup("Antelope");
    if(err != DB_SUCCESS){
        puts(ib_strerror(err));
        return err;
    }

    ib_trx_t trx;
    ib_crsr_t crsr;
    trx=ib_trx_begin(IB_TRX_REPEATABLE_READ);
    assert(trx != NULL);
    err=ib_cursor_open_table(dtname,trx,&crsr);
    if(err != DB_SUCCESS){
        puts(ib_strerror(err));
        return err;
    }
    ib_tpl_t tpl;
    tpl=ib_clust_read_tuple_create(crsr);
    assert(tpl != NULL);
    
    err=ib_cursor_first(crsr);
    while(err == DB_SUCCESS){
        err=ib_cursor_read_row(crsr,tpl);
        print_tuple(stdout,tpl);
        err=ib_cursor_next(crsr);
        tpl=ib_tuple_clear(tpl);
    }
    ib_tuple_delete(tpl); 
    err=ib_cursor_close(crsr);
    if(err != DB_SUCCESS){
        puts(ib_strerror(err));
        return err;
    }
    err=ib_trx_commit(trx);
    if(err != DB_SUCCESS){
        puts(ib_strerror(err));
        return err;
    }
    
    err=ib_shutdown(IB_SHUTDOWN_NORMAL);
    if(err != DB_SUCCESS){
        puts(ib_strerror(err));
        return err;
    }
    return 0;
}
