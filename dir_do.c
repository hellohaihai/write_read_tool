#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>


/**
 *  funtion: write,read,mkdir for cluster test
 *  jiehongui@estor.com.cn
 *  2018-05-18
 *  
 *
 * */


#define DIRNAME "/cluster2/test"
#define THREAD_NUM 20
#define READ 0
#define WRITE 1
#define MKDIR 2

int opendirs(const char *,int,int);
int mkdirs(const char *);
int nIndex[100] ={ 0 };
char *mountDir[] = {"/cluster/test","/dir1","/dir2","/dir3","/dir4","/dir5","/dir6","/dir7"};
int thread_num = 0;
int type = 0;

struct option {
    int index;
    int file_size;
    int file_num;
    char dirname[128];
};

void *my_log(void *data)
{
	int preIndex[100]= {0},i = 0, flag[100] = {0};
	FILE *fp = NULL;
	time_t tBegin = time(0), tEnd = time(0);
	char tmpBuf[30];
	time_t t;
	char resu[512];
	char result[2048];
	int file_num_one = 0,file_num_all = 0;
	char file_name[256];

	tEnd = time(0);
	memset(tmpBuf,0,sizeof(tmpBuf));
	memset(file_name,0,sizeof(file_name));
	strftime(tmpBuf, sizeof(tmpBuf), "%Y-%m-%d_%H-%M-%S", localtime(&tEnd));
	if(type == READ){
	    sprintf(file_name,"%s_%s",tmpBuf,"read");
	}else if(type == WRITE){
	    sprintf(file_name,"%s_%s",tmpBuf,"write");
	}else if(type == MKDIR){
	    sprintf(file_name,"%s_%s",tmpBuf,"mkdir");
	}

	while(1)
	{
		t = time(NULL);
		tEnd = time(0);
		if ((tEnd - tBegin) >= 600)
		{
			for(i = 0 ; i < thread_num ; i++){
				printf("nIndex[%d]=%d,preIndex[%d]=%d\n",i,nIndex[i],i,preIndex[i]);
			}
			memset(tmpBuf,0,sizeof(tmpBuf));
			memset(resu,0,sizeof(resu));
			memset(result,0,sizeof(result));
			strftime(tmpBuf, sizeof(tmpBuf), "%Y-%m-%d %H:%M:%S", localtime(&tEnd));
			
			for(i = 0 ; i < thread_num ; i++)
			{
				file_num_one = nIndex[i] - preIndex[i];
				file_num_all += file_num_one ;
				sprintf(resu,"date:%s,nIndex =%d,Ten minutes read:%d, time =%d s\n",tmpBuf,nIndex[i],file_num_one,(tEnd - tBegin));
				preIndex[i] = nIndex[i];
				strcat(result,resu);
				memset(resu,0,sizeof(resu));
				file_num_one = 0 ;
			}
			sprintf(resu,"ten min %d thread read all:%d\n\n",thread_num,file_num_all);
			strcat(result,resu);
			fp = fopen(file_name,"a+");
			if (NULL == fp){
				printf("openfail:%s\n",strerror(errno));
				continue;
			}
			fwrite(result,strlen(result),1,fp);
			fclose(fp);
			tBegin = tEnd;
			file_num_all = 0;
		}
		if ((tEnd - tBegin) >= 5)
		{
			if((flag[0] == nIndex[0]) && (flag[thread_num-1] == nIndex[thread_num-1]))
				break;
			for(i = 0 ; i < thread_num ; i++)
			{
				flag[i] = nIndex[i];
			}
		}
	}
	
}
void *my_read(void *data)
{
	int ret = 0;
	struct option *option = data;
	int index = option->index;

reload:
	//printf("the thread ID is %lld,index = %d\n",pthread_self(),index);	
	ret = opendirs(option->dirname,index,index);
	if(ret == 1){
		printf("open dir fail ,please check dirname\n");
	}
	goto reload;
	
}
void *my_write(void *data){
	int ret = 0,i;
	struct option *option = data;
	int index = option->index;
	unsigned long  threadID = pthread_self();
	FILE *fp = NULL;
	time_t t;
	struct tm* pTime;
	char tmpBuf[30];
	char buf[1024];
	char szFilePath[256] = {0};
	char szFileName[256] = { 0 };

	for(i = 0 ; i<sizeof(buf);i++){
	    buf[i] = 'a';
	}

	while(nIndex[index] <= (option->file_num)){
	    t = time(NULL);
	    pTime = gmtime(&t);
	    
	    memset(szFilePath,0,sizeof(szFilePath));
	    memset(szFileName,0,sizeof(szFileName));

	    sprintf(szFilePath, "%s/%lld/%04d/%02d/%02d/%02d",
		    option->dirname, threadID, pTime->tm_year + 1900, pTime->tm_mon + 1, pTime->tm_mday, pTime->tm_hour);
	    sprintf(szFileName, "%s/%d.jpg", szFilePath, nIndex[index]);
reset:
	    fp = fopen(szFileName, "w+");
	    if(fp){
		    for(i = 0 ; i < option->file_size;i++){
			fwrite(buf,sizeof(buf),1,fp);
		    }
		    fclose(fp);
		    fp = NULL;
		    printf("create file %s sucessful\n ",szFileName);
		    nIndex[index] ++;
	    }else{
		if(2 == errno){
		    printf("create file %s fail,because %s\n",szFileName,strerror(errno));
		    mkdirs(szFilePath);
		    goto reset;
		}
	    }
	}
}
void *my_mkdir(void *data){
    
}
int main(int argc,char **argv)
{
	int ret = 0,i = 0;
	void *result;
	pthread_t log_thread ;//记录线程
	pthread_t read_thread[100];//运行线程
	struct option thread_option[100];//线程参数
	void* (*function)(void *);


	//检查参数同时判断操作类型，read|write|mkdir
	if(argc <= 2){
	    fprintf(stderr,"please check args :%s\n\t%s\n\t%s\n\t%s\n\t%s\n\t%s\n",
		    argv[0],"测试类型(read|write|mkdir)","线程数","路径","文件大小","文件数量");
	    return -1;
	}else if(!strcmp(argv[1],"read")){
	    type = READ;//读文件
	}else if(!strcmp(argv[1],"write")){
	    type = WRITE;//写文件
	}else if(!strcmp(argv[1],"mkdir")){
	    type = MKDIR;//创建文件夹
	}else{
	    fprintf(stderr,"please check args,no option :%s\n",argv[1]);
	    return -1;
	}

	thread_num = atoi(argv[2]);
	
	for(i = 0 ; i < thread_num ; i++){
	    thread_option[i].index = i;
	    if(type == WRITE){
		//若为写文件则初始化文件大小和数量
		if(argc < 6){
		    fprintf(stderr,"please check args the option write need 线程数 路径 文件大小 文件数量\n");
		    return -1;
		}
		thread_option[i].file_size = atoi(argv[4]);
		thread_option[i].file_num = atoi(argv[5]);
		function = my_write;
	    }else if(type == READ){
		if(argc < 4 ){
		    fprintf(stderr,"please check args the option read need 线程数 路径\n");
		    return -1;
		}
		//若为读文件则初始化文件大小和数量为0
		thread_option[i].file_size = 0;
		thread_option[i].file_num = 0;
		function = my_read;
	    }else if(type == MKDIR){
		if (argc < 4){
		    fprintf(stderr,"please check args the option mkdir need 线程数 路径\n");
		    return -1;
		}
		thread_option[i].file_size = 0;
		thread_option[i].file_num = 0;
		function = my_mkdir;
	    }else{
		fprintf(stderr,"please check args,no option :%s\n",argv[1]);
		return -1;
	    }
	    memset(thread_option[i].dirname,0,sizeof(thread_option[i].dirname));
	    strcpy(thread_option[i].dirname,argv[3]);
	}

	for(i = 0 ; i < thread_num ; i++)
	{
		ret = pthread_create(&read_thread[i],NULL,function,(void *)(&thread_option[i]));
		if(ret){
			printf("read_thread create fail \n");
			exit(-1);
		}
		//sleep(2);
	}

	ret = pthread_create(&log_thread,NULL,my_log,NULL);
	if(ret){
		printf("log_thread create fail \n");
		exit(-1);
	}

	for(i = 0 ; i < thread_num ; i++){
		pthread_join(read_thread[i],&result);
	}

	pthread_join(log_thread,&result);


	return 0;
}
int opendirs(const char *data,int index,int count){
	DIR *dir;
	struct dirent * ptr;
	char dirname[256];
	char filename[512];
	int ret = 0;
	FILE *fp = NULL;
	char gf_buf[1024];
	int flag = 0;

	memset(dirname,0,sizeof(dirname));
	strcpy(dirname,data);

	dir = opendir(dirname);
	if (dir == NULL){
		printf("error:%s\n",strerror(errno));
		return 1;
	}else{
		while((ptr = readdir(dir)) != NULL){
			if((strcmp(ptr->d_name,".") == 0) || (strcmp(ptr->d_name,"..") == 0) || (strcmp(ptr->d_name,"home") == 0))
				continue;
			if(count){
				count --;
				continue;
			}
			if( flag == 0){	
				strcat(dirname,"/");
				strcat(dirname,ptr->d_name);
				ret = opendirs(dirname,index,count);
				if(ret == 1)
					flag = 1;
				else
					flag = 0;
			}else{
				memset(filename,0,sizeof(filename));
				memset(gf_buf,0,sizeof(gf_buf));
				sprintf(filename,"%s/%s",data,ptr->d_name);
				fp = fopen(filename,"r");
				if(fp == NULL ){
					printf("open file fail %s\n",strerror(errno));
					continue;
				}
				while( fread(gf_buf,sizeof(gf_buf),1,fp) > 0){
					memset(gf_buf,0,sizeof(gf_buf));
				}
				//fread(gf_buf,1,sizeof(gf_buf),fp);
				fclose(fp);
				printf("filename is %s\n",filename);
				nIndex[index]++ ;
			}
			memset(dirname,0,sizeof(dirname));
			strcpy(dirname,data);
		}
		closedir(dir);
	}
	return 0;
}
int mkdirs(const char *dir){
    char str[1024];
    int ret = 0,i,num = 0;
    DIR *dirFP;

    if (strlen(dir) == 0 || dir == NULL){
	printf("strlen(dir) is 0 or dir is NULL./n");
	ret = -1;
	return ret;
    }

    memset(str,0,sizeof(str));

    dirFP = opendir(dir);
    if(NULL==dirFP){
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
	closedir(dirFP);
	dirFP = NULL;
	printf("---open sucess ----%s \n",dir);
	return 0;
    }
}
