#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
/*
 * to create small file 
 * argv[1] : pthread num
 * argv[2] : file num
 * argv[3] : file size
 * argv[4] : dir path
 * argv[5] : file_name
 *
 * author : 研发二部 解宏辉/xiehonghui@estor.com.cn
 * date : 2018-01-30
 *
 * */
void *create_file(void *);
int mkdirs(const char *);
struct option {
    int pthread_num ;
    int file_num ;
    int file_size ;
    char dir_name[100];
    char file_str[100];
};
struct pthread_option{
    int file_num_start;
    int file_num_end;
    int file_size;
    char filename[50];
};
int main(int argc,char **argv)
{
    char *arg[] = {"请检查参数","argv[1] : pthread num(线程数量)",
                   "argv[2] : file num(文件总数)",
                   "argv[3] : file size(单个文件大小)",
                   "argv[4] : dir path(创建文件路径)",
                   "argv[5] : file_name(文件名)"};
    struct option num;
    struct pthread_option p_num[300];
    pthread_t t[300];
    void * result;
    int i = 0;

    if(argc != 6){
        printf("%s:\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n",argv[0],arg[0],arg[1],arg[2],arg[3],arg[4],arg[5]);
        exit(1);
    }

    memset(num.file_str,'\0',sizeof(num.file_str));
    memset(num.dir_name,'\0',sizeof(num.dir_name));
    num.pthread_num = atoi(argv[1]);
    num.file_num = atoi(argv[2]);
    num.file_size = atoi(argv[3]);
    strcpy(num.dir_name,argv[4]);
    strcat(num.file_str,num.dir_name);
    strcat(num.file_str,"/");
    strcat(num.file_str,argv[5]);
    
    if(NULL==opendir(num.dir_name)){
        printf("dir not exist,create dir.\n");
        if(mkdirs(num.dir_name) < 0){
            printf("create dir fail,please check path:%s\n",num.dir_name);
        }else{
            printf("%s create sucessful\n",num.dir_name);
        }
    }else{
        printf("dir already exist.\n");
    }
    

    //printf("argv= %d  %d  %d\n",num.pthread_num,num.file_num,num.file_size);
    p_num[0].file_num_start = 1;
    p_num[0].file_num_end = 1 + num.file_num/num.pthread_num;
    p_num[0].file_size = num.file_size;
    strcpy(p_num[0].filename,num.file_str);

    for(i=1;i<num.pthread_num;i++){
        p_num[i].file_num_start = p_num[i-1].file_num_end;
        p_num[i].file_num_end = p_num[i].file_num_start + num.file_num/num.pthread_num;
        p_num[i].file_size = num.file_size;
        strcpy(p_num[i].filename,num.file_str);
        //printf("p_num %d= %d  %d  %d\n",i,p_num[i].file_num_start,p_num[i].file_num_end,p_num[i].file_size);
    }
    
    for(i=0;i<num.pthread_num;i++){
        if(pthread_create(&t[i], NULL,create_file, (void *)&p_num[i]) == -1){
            printf("fail to create pthread t0\n");
            exit(1);
        }
    }

    
    for (i=0;i<num.pthread_num;i++){
        if(pthread_join(t[i], &result) == -1){
            puts("fail to recollect t0");
            exit(1);
        }
    }
    
    return 0;

}
void *create_file(void *num){
    int fd,i;
    struct pthread_option *option = (struct pthread_option *)num;
    struct stat statbuff;
    char str[100];
    char filename[100];
    
    strcpy(filename,option->filename);
    for(i = option->file_num_start;i < option->file_num_end ; i++){
        sprintf(str,"%d",i);
        strcat(filename,str);
        printf("start create file :%s thread_id : %lu \n",filename,pthread_self());
        fd = open(filename,O_RDWR|O_CREAT,0644);
        if( fd < 0 ){
            printf("open or create file fail.exit,%s\n",strerror(errno));
            exit(1);
        }
        while(1){
            stat(filename,&statbuff);
            write(fd,filename,strlen(filename));
            if(statbuff.st_size >= option->file_size){
                break;
            }
        }
        close(fd);
        strcpy(filename,option->filename);
    }
    
}
int mkdirs(const char *dir){
    char str[1024];
    int ret = 0,i,num = 0;

    if (strlen(dir) == 0 || dir == NULL) {
        printf("strlen(dir) is 0 or dir is NULL./n");
        ret = -1;
        return ret;
    }

    memset(str,0,sizeof(str));

    if(NULL==opendir(dir)){
        printf("%s opendir fail.\n",dir);
        for(i=0;i<strlen(dir);i++){
            if(dir[i] == '/')
                num++;
        }
        if(num <= 1){

            ret = mkdir(dir,0775);
            if(ret < 0)
                printf("create dir %s faild\n",dir);
            else
                printf("create dir %s sucessful\n",dir);
            return ret;
        }
        for(i=strlen(dir);i>=0;i--){
            if(dir[i] == '/'&& i != (strlen(dir)-1)){
                strncpy(str,dir,i);
                if(mkdirs(str) < 0){
                    return -1;
                }else{
                    ret =  mkdir(dir,0775);
                    if(ret < 0)
                        printf("create dir %s faild\n",dir);
                    else
                        printf("create dir %s sucessful\n",dir);
                    return ret;
                }
            }
        }
    }else{
        printf("---open sucess ----%s \n",dir);
        return 0;
    }

}













