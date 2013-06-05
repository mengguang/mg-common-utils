#include <stdio.h>
#include <apr.h>
#include <apr_pools.h>
#include <apr_errno.h>
#include <apr_strings.h>
#include <apr_md5.h>
#include <apr_file_io.h>
#include <apr_mmap.h>

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

    char *filename;
    if(argc > 1) {
        filename=argv[1];
    } else {
        filename="apr_mmap.c";
    }

    apr_file_t *file;
    st=apr_file_open(&file,filename,APR_READ,APR_REG,pool);
    if(st != APR_SUCCESS) {
        apr_err("apr_file_open()",st);
        return st;
    }

    apr_mmap_t *mm;
    apr_finfo_t info;
    apr_file_info_get(&info,0,file);
    st=apr_mmap_create(&mm,file,0,info.size,APR_MMAP_READ,pool);
    if(st != APR_SUCCESS) {
        apr_err("apr_mmap_create()",st);
        return st;
    }

    apr_file_close(file); 

    void *p;
    apr_mmap_offset(&p,mm,0);

    unsigned char result[APR_MD5_DIGESTSIZE];
    apr_md5(result,p,info.size);
    apr_mmap_delete(mm);
    
    for(int i=0;i<APR_MD5_DIGESTSIZE;i++) {
        printf("%02x",result[i]);   
    }
    printf("\n");
    

    apr_pool_destroy(pool);
    apr_terminate();
    return 0;
}

