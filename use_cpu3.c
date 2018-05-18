#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>

void * use_a(void *);


int main()
{
    pthread_t t[12];
    
    int i = 0;
    
    for (i=0;i<12;i++)
    {
        if(pthread_create(&t[i], NULL, use_a, NULL) == -1){
            printf("fail to create pthread t0");
            exit(1);
        }
    }
    
    void * result;
    for (i=0;i<12;i++){
        if(pthread_join(t[i], &result) == -1){
        puts("fail to recollect t0");
        exit(1);
    }
    }
}
void * use_a(void *a)
{
    double i = 6.66 , j = 8.88;
    while(1)
    {
        i = pow(i,j);
        j = pow(j,i);
        i = pow(i,j);
        j = pow(j,i);
        i = pow(i,j);
        j = pow(j,i);
        i = pow(i,j);
        j = pow(j,i);
    }
    	
}
