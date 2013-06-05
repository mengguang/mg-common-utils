#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

typedef struct FAT_FS {
    short BytsPerSec;
    char SecPerClus;
    short RsvdSecCnt;
    char NumFATs;
    int TotSec32;
    int FATSz32;
    int RootClus;

    int cluster_size;
    int *fat_table;
    size_t cluster_begin_addr;
    FILE *fp;

} FAT_FS;
    

FAT_FS *fat_fs_open(char *path) {
    FAT_FS *fs=calloc(1,sizeof(FAT_FS));
    fs->fp=fopen(path,"r");
    if(!fs->fp) {
        perror("fopen");
        return NULL;
    }
    fseek(fs->fp,11L,SEEK_SET);
    fread(&(fs->BytsPerSec),1,2,fs->fp);

    fseek(fs->fp,13L,SEEK_SET);
    fread(&(fs->SecPerClus),1,1,fs->fp);

    fseek(fs->fp,14L,SEEK_SET);
    fread(&(fs->RsvdSecCnt),1,2,fs->fp);

    fseek(fs->fp,16L,SEEK_SET);
    fread(&(fs->NumFATs),1,1,fs->fp);

    fseek(fs->fp,32L,SEEK_SET);
    fread(&(fs->TotSec32),1,4,fs->fp);

    fseek(fs->fp,36L,SEEK_SET);
    fread(&(fs->FATSz32),1,4,fs->fp);

    fseek(fs->fp,44L,SEEK_SET);
    fread(&(fs->RootClus),1,4,fs->fp);

    fs->cluster_size=fs->BytsPerSec * fs->SecPerClus;
    fs->fat_table=calloc(1,fs->FATSz32 * fs->BytsPerSec);

    fseek(fs->fp,fs->RsvdSecCnt * fs->BytsPerSec,SEEK_SET);
    fread(fs->fat_table,1,fs->FATSz32 * fs->BytsPerSec,fs->fp);

    fs->cluster_begin_addr=fs->RsvdSecCnt * fs->BytsPerSec + fs->FATSz32 * fs->BytsPerSec * 2;

    return fs;
}

void fat_fs_close(FAT_FS *fs) {
    if(fs == NULL) return;
    fclose(fs->fp);
    if(fs->fat_table != NULL) {
        free(fs->fat_table);
    }
    free(fs);
    return;
}

int fat_cluster_get_next(FAT_FS *fs, int cluster_num) {
    int nextClus=(fs->fat_table[cluster_num] & 0xFFFFFFF) ;
    if(nextClus >= 0xFFFFFF8) {
        return -1;
    } else {
        return nextClus;
    }
}

char *fat_cluster_read(FAT_FS *fs,int num,char *buf) {
    if(buf == NULL) return NULL;
    long pos=fs->cluster_begin_addr + (num -2) * fs->cluster_size;
    printf("pos : %ld\n",pos);
    if(pos != ftell(fs->fp)) {
        fseek(fs->fp,fs->cluster_begin_addr + (num -2) * fs->cluster_size,SEEK_SET);
    }
    fread(buf,1,fs->cluster_size,fs->fp);
    return buf;
}


typedef struct FAT_FILE_INFO {
    char short_name[12];
    char attr;
    int ctime;
    int mtime;
    int size;
    char is_dir;
    char is_read_only;
    char is_hidden;
    char is_system;
    char is_long_name;
    
} FAT_FILE_INFO;

typedef struct FAT_FILE {
    FAT_FS *fs;
    FAT_FILE_INFO info;
    int first_cluster;
    int current_cluster;
    int current_pos;
} FAT_FILE;

FAT_FILE *fat_file_open(FAT_FS *fs,int first_cluster) {
    FAT_FILE *file=calloc(1,sizeof(FAT_FILE));
    file->fs=fs;
    file->first_cluster=first_cluster;
    file->current_cluster=first_cluster;
    file->current_pos=0;
    return file;
}

void fat_file_close(FAT_FILE *file) {
    if(file != NULL) {
        free(file);
    }
}

void fat_file_seek(FAT_FILE *file,size_t pos) {
    

}

size_t fat_file_read(FAT_FILE *file,char *buf,size_t n) {

}

typedef struct FAT_DIR {
    FAT_FS *fs;
    int first_cluster;
    int current_cluster;
    int current_pos;
} FAT_DIR;

FAT_DIR *fat_dir_open(FAT_FS *fs,int first_cluster) {
    FAT_DIR *dir=calloc(1,sizeof(FAT_DIR));
    dir->fs=fs;
    dir->first_cluster=first_cluster;
    dir->current_cluster=first_cluster;
    dir->current_pos=0;
    return dir;
}

void fat_dir_close(FAT_DIR *dir) {
    if(dir != NULL) {
        free(dir);
    }
}

void fat_dir_rewind(FAT_DIR *dir) {
    dir->current_pos=0;
    dir->current_cluster=dir->first_cluster;
}

FAT_FILE *fat_dir_read(FAT_DIR *dir) {
    char cluster[dir->fs->cluster_size];
    fat_cluster_read(dir->fs,dir->current_cluster,cluster);
    char *p=cluster+dir->current_pos;
    dir->current_pos+=32;
    FAT_FILE *file=calloc(1,sizeof(FAT_FILE));
    while(*p != '\0' && *p != 0xE5 && *p != 0x05) {
        file->info.attr=*(p+11);
        if(file->info.attr & 15 == 15) {
            p+=32;
            dir->current_pos+=32;
            continue;
        }
        memset(file->info.short_name,0,sizeof(file->info.short_name));
        strncpy(file->info.short_name,p,11);
        if(file->info.attr & 16 ) {
            file->info.is_dir=1;
        } else {
            file->info.is_dir=0;
        }
        short hi=*(short *)(p+0x14);
        short lo=*(short *)(p+0x1a);
        file->first_cluster = 256 * hi + lo;
        file->info.size=*((int*)(p+28));
        return file;
    }
    return NULL;
}


char *fat_file_get_contents(FAT_FILE *file) {
        int next_cluster;
        int cluster=file->first_cluster;
        char *buf=calloc(1,((file->info.size + file->fs->cluster_size -1 )/file->fs->cluster_size ) * file->fs->cluster_size);
        char pos=0;
        for(;;){
            fat_cluster_read(file->fs,cluster,buf+pos);
            pos+=file->fs->cluster_size;
            next_cluster=fat_cluster_get_next(file->fs,cluster);
            if(next_cluster == -1) {
                break;
            } else {
                cluster=next_cluster;
            }
        }
        return buf;
}

int main(int argc,char **argv) {
    char *path="fat1g";
    if(argc > 2) {
        path=argv[1];
    }

    FAT_FS *fs=fat_fs_open(path);
    FAT_DIR *dir=fat_dir_open(fs,2);
    FAT_FILE *file;
    while((file=fat_dir_read(dir)) != NULL) {
        printf("name => %s\t is_dir => %d\t size => %d\n",file->info.short_name,file->info.is_dir,file->info.size);
    }
    fat_dir_close(dir);
    fat_fs_close(fs);

    return 0;
}

