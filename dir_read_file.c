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
#include<time.h>
#include <sys/time.h>
#include <dirent.h>


int main()
{
    //char *DirName = "/cluster2/feisou/data2/small/2018-07-10/08";
    char *DirName = "/cluster2/test/";
    char *file = "file";
    int i = 0,fd;
    DIR * dir = NULL;
    DIR * dir1 = NULL;
    struct dirent * ptr = NULL;
    struct dirent * ptr1 = NULL;
    char dirname[512];
    char filename[1024];
    char bufsize[1024];
    struct stat s_buf;
    struct stat s_buf1;
    struct timeval tv; 
    struct timeval tv1;

    dir = opendir(DirName);
    if(dir==NULL){
        printf("please check path\n");
        exit(-1);
    }   
    while((ptr = readdir(dir)) != NULL){
        if(i==10000){
            break;
        }   
        if(!strcmp("..",ptr->d_name)||!strcmp(".",ptr->d_name)){
            continue;
        }   
        memset(dirname,'\0',sizeof(dirname));
        sprintf(dirname,"%s/%s",DirName,ptr->d_name);
        //printf("dirname=%s\n",dirname);
        stat(dirname,&s_buf);
        if(S_ISDIR(s_buf.st_mode)){
            //printf("dirname=%s\n",ptr->d_name);
            dir1 = opendir(dirname);
            if(dir1==NULL){
                perror("opendir error");
                continue;
            }   
            //printf("haihai\n");
            while((ptr1 = readdir(dir1)) != NULL){
                if(!strcmp("..",ptr1->d_name)||!strcmp(".",ptr1->d_name)){
                    continue;
                }                
                //printf("haihai1\n");
                memset(filename,'\0',sizeof(filename));                
                sprintf(filename,"%s/%s",dirname,ptr1->d_name);
                //printf("filename=%s\n",filename);                
                stat(filename,&s_buf1);
                if(S_ISREG(s_buf1.st_mode)){                    
                    gettimeofday(&tv,NULL);
                    fd = open(filename,O_RDONLY);
                    if(fd < 0){
                        perror("open file fail");
                        continue;
                    }
                    while(read(fd,bufsize,sizeof(bufsize)) > 0){
                        memset(bufsize,0,sizeof(bufsize));
                    }
                    close(fd);
                    gettimeofday(&tv1,NULL);
                    printf("filename=%s use second:%ld,use millisecond:%ld,use microsecond:%ld\n",filename,tv1.tv_sec-tv.tv_sec,(tv1.tv_sec*1000 + tv1.tv_usec/1000)-(tv.tv_sec*1000 + tv.tv_usec/1000),(tv1.tv_sec*1000000 + tv1.tv_usec)-(tv.tv_sec*1000000 + tv.tv_usec));
                    i++;
                }else{
                    continue;
                }
            }
            closedir(dir1);
        }else{
            continue;
        }
        //printf("dirname=%s\n",ptr->d_name);
    }
    closedir(dir);




}
