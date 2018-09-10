#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>
#include<time.h>
/*
 * to mkdir 
 * argv[1] : pthread num
 * argv[2] ；dir num
 * argv[3] : Operating path
 *
 * author : 研发二部 解宏辉/xiehonghui@estor.com.cn
 * date : 2018-08-30 09:08
 *
 * */

int mkdirs(const char *);
struct pthread_option {
    int dir_num;
    char dir_name[512];
};
struct date {
    int year;
    int mouth;
    int day;
    int hour;
};

void *my_mkdir(void *aaa)
{
    struct pthread_option *option = (struct pthread_option *)aaa;
    char dirname[1024];
    struct date today;
    int i = 1,ret = 0 ,num = 0;

    printf("%d %s\n",option->dir_num,option->dir_name);
    memset(dirname,'\0',sizeof(dirname));

    //sprintf(dirname,"%s/%lu",option->dir_name,pthread_self());

    for(today.year = 2018;;today.year++){
        for(today.mouth = 1 ;today.mouth < 13 ;today.mouth ++){
            for(today.day = 1; today.day < 31 ;today.day ++){
                i++;
                for(today.hour = 1 ;today.hour < 25 ; today.hour ++){
                    i++;
                    for(num =1 ;num<=300;num++){
                        sprintf(dirname,"%s/%lu/%d-%d-%d/%d/%s%d",option->dir_name,pthread_self(),today.year,today.mouth,today.day,today.hour,"qwert",i);
                        ret = mkdirs(dirname);
                        if(ret<0){
                            continue;
                        }else{
                            i++;
                        }
                        if(i>=option->dir_num)
                            goto OUT;
                    }
                }
            }
        }
    }
OUT:
    printf("thread id %lu mkdir over\n",pthread_self());    
    
}

int main(int argc,char **argv){

    char *arg[] = {"请检查参数：","argv[1] : pthread num(线程数量)",
                    "argv[2] : dir num(单线程创建目录数)",
                    "argv[3] : dir path(创建目录路径)"};
    struct pthread_option option;
    int pthread_num = 0,i = 0;
    void * result;
    DIR *ptr;
    pthread_t t[100];

    if(argc != 4){
        printf("%s:\n \t%s\n \t%s\n \t%s\n",arg[0],arg[1],arg[2],arg[3]);
        exit(-1);
    }
    
    memset(option.dir_name,'\0',sizeof(option.dir_name));
    
    /*初始化参数*/
    pthread_num = atoi(argv[1]);
    option.dir_num = atoi(argv[2]);
    strcpy(option.dir_name,argv[3]);

    printf("%d %d %s\n",pthread_num,option.dir_num,option.dir_name);

    ptr=opendir(option.dir_name);
    if(ptr==NULL){
        printf("dir not exist,create dir.\n");
        if(mkdirs(option.dir_name) < 0){
            printf("create dir fail,please check path:%s\n",option.dir_name);
            exit(-1);
        }else{
            printf("%s create sucessful\n",option.dir_name);
        }
    }else{
        closedir(ptr);
        printf("dir already exist.\n");
    }

    /*创建线程*/
    for(i=0;i<pthread_num;i++){
        if(pthread_create(&t[i], NULL,my_mkdir, (void *)&option) == -1){
            printf("fail to create pthread t0\n");
            exit(-1);
        }
    }

    for (i=0;i<pthread_num;i++){
        if(pthread_join(t[i], &result) == -1){
            printf("fail to recollect t[%d]\n",i);
            continue;
        }
    }
    return 0;
}

int mkdirs(const char *dir){
    char str[1024];
    int ret = 0,i,num = 0;
    struct timeval tv;
    struct timeval tv1;

    if (strlen(dir) == 0 || dir == NULL) {
        printf("strlen(dir) is 0 or dir is NULL./n");
        ret = -1;
        return ret;
    }

    memset(str,0,sizeof(str));

    if(NULL==opendir(dir)){
        //printf("%s opendir fail.\n",dir);
        for(i=0;i<strlen(dir);i++){
            if(dir[i] == '/')
                num++;
        }
        if(num <= 1){
            gettimeofday(&tv,NULL);
            ret = mkdir(dir,0775);
            if(ret < 0){
                //printf("create dir %s faild\n",dir);
            }else{
                gettimeofday(&tv1,NULL);
                printf("dirname=%s use second:%ld,use millisecond:%ld,use microsecond:%ld    \n",
                        dir,tv1.tv_sec-tv.tv_sec,(tv1.tv_sec*1000 + tv1.tv_usec/1000)-(tv.tv_sec*1000 + tv.tv_usec/1000),
                        (tv1.tv_sec*1000000 + tv1.tv_usec)-(tv.tv_sec*1000000 + tv.tv_usec));
                //printf("create dir %s sucessful\n",dir);
            }
            return ret;
        }
        for(i=strlen(dir);i>=0;i--){
            if(dir[i] == '/'&& i != (strlen(dir)-1)){
                strncpy(str,dir,i);
                if(mkdirs(str) < 0){
                    return -1;
                }else{
                    gettimeofday(&tv,NULL);
                    ret =  mkdir(dir,0775);
                    if(ret < 0){
                        //printf("create dir %s faild\n",dir);
                    }else{
                        //printf("create dir %s sucessful\n",dir);
                        gettimeofday(&tv1,NULL);
                        printf("dirname=%s use second:%ld,use millisecond:%ld,use microsecond:%ld    \n",
                                dir,tv1.tv_sec-tv.tv_sec,(tv1.tv_sec*1000 + tv1.tv_usec/1000)-(tv.tv_sec*1000 + tv.tv_usec/1000),
                                (tv1.tv_sec*1000000 + tv1.tv_usec)-(tv.tv_sec*1000000 + tv.tv_usec));
                    }
                    return ret;
                }
            }
        }
    }else{
        //printf("---open sucess ----%s \n",dir);
        return 0;
    }

}
