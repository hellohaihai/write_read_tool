#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
void * write_test(void *);
int pthread_num = 100,file_num = 100000000,file_size = 512000;
int main()
{
    pthread_t t[pthread_num];
    
    int i = 0;
    int num[pthread_num];
    num[0]=1;
    for(i=1;i<pthread_num;i++)
    {
        num[i]=num[i-1]+file_num/pthread_num;
    }
    for(i=0;i<pthread_num;i++){
        if(pthread_create(&t[i], NULL, write_test, (void *)&num[i]) == -1){
            printf("fail to create pthread t0");
            exit(1);
        }
    }
    void * result;
    for (i=0;i<pthread_num;i++){
        if(pthread_join(t[i], &result) == -1){
        puts("fail to recollect t0");
        exit(1);
        }
    }
    return 0;
}
void * write_test(void *a)
{
    int fd;
    char filename[50] = "/cluster2/test/haihai/test";
    char str[10];
    int i = *(int *)a;
    int len = i+file_num/pthread_num;
    struct stat statbuff;
    
    for(i;i<len;i++)
    {
        sprintf(str,"%d",i);
        strcat(filename,str);
        fd = open(filename,O_RDWR|O_CREAT,0644);
        while(1){
            write(fd,filename,strlen(filename));
            stat(filename,&statbuff);
            if(statbuff.st_size >= file_size){
                break;
            }
       }
        close(fd);
        sprintf(filename,"/cluster2/test/haihai/test");
    }
}
