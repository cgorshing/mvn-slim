//https://github.com/ankushagarwal/nweb/blob/master/nweb23.c
//
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <malloc/malloc.h>
#include <fcntl.h>        
#include <sys/stat.h> 
#include <time.h>
#include <semaphore.h>

#define BUF_SIZE 1024

#include "main.h"

#define OK_IMAGE    "HTTP/1.0 200 OK\nContent-Type:image/gif\n\n"
#define OK_TEXT     "HTTP/1.0 200 OK\nContent-Type:text/html\n\n"
#define NOTOK_404   "HTTP/1.0 404 Not Found\nContent-Type:text/html\n\n"
#define MESS_404    "<html><body><h1>FILE NOT FOUND</h1></body></html>"

void *thread_serve()
{
  while(1)
  {    
    log_msg("entered serving thread");
    pthread_mutex_lock(&sthread_mutex);    
    pthread_cond_wait(&cond_var,&sthread_mutex);
    
    log_msg("got signal");
    //wait on condition mutex

    struct request r=r2;
    pthread_mutex_unlock(&sthread_mutex);
    log_msg("serving thread unlocked sthread_mutex");
    
    time_t now;          // getting the time the job has been assigned to the serving thread
    time(&now);
    struct tm * ct=localtime(&now); //getting localtime
    char ch[128], time_serve[128];
    struct timeval tv;
    strftime(ch, sizeof ch, "[%d/%b/%Y : %H:%M:%S %z]", ct); //format of the timestamp string we need
    snprintf(time_serve, sizeof time_serve, ch, tv.tv_usec); //printing the needed timestamp string
    
    unsigned int ip=r.cli_ipaddr;
  
    /* code adapted from stackoverflow.com */
    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;
    /* end of code adapted from stackoverflow.com */
    
    //struct request r= *((struct request *)arg);
    if(debug_flag==0&& log_flag==1)
    {
      FILE * file_des=fopen(file,"a"); 
      log_msg("in serving thread");
    
      fprintf(file_des,"%d.%d.%d.%d\t-\t ", bytes[0], bytes[1], bytes[2], bytes[3]);
      fprintf(file_des,"%s\t %s\t %s \t status\t %d\n",r.time_arrival,time_serve,r.in_buf,r.size);
  
      fclose(file_des);
    }
    else if(debug_flag==1)
    {  
      printf("%d.%d.%d.%d\t-\t %s\t %s\t %s \t status\t %d\n", bytes[0], bytes[1], bytes[2], bytes[3],r.time_arrival,time_serve,r.in_buf,r.size);
      //fprintf(stdout,"%s\t %s\t %s \t status\t %d\n",r.time_arrival,time_serve,r.in_buf,r.size);
    }
  
    log_msg("in serving thread copied structure");

    char           in_buf[BUF_SIZE];      
    char           out_buf[BUF_SIZE];
    char           *file_name;                 
    file_name=malloc(sizeof(char *));
    int acceptfd;
    unsigned int   fd1;                         
    unsigned int   buffer_length;                    
    unsigned int   retcode;  
    int m; 

    log_msg("in serving thread before copying variables");
    acceptfd=r.acceptfd;
    file_name=r.file_name;
    log_msg("in serving thread after copying variables");
                
    printf("in serving thread file name is %s\n",file_name);
    {  
      log_msg("in serving thread opening file");
    
      /* This part of code adopted from http://kturley.com/simple-multi-threaded-web-server-written-in-c-using-pthreads/ */

      fd1 = open(&file_name[1], O_RDONLY, S_IREAD | S_IWRITE);
  
      memset(out_buf, 0, sizeof(out_buf));
   
      if (fd1 == -1)
      {
        printf("File %s not found - sending an HTTP 404 \n", &file_name[1]);
        strcpy(out_buf, NOTOK_404);
        send(acceptfd, out_buf, strlen(out_buf), 0);
        strcpy(out_buf, MESS_404);
        send(acceptfd, out_buf, strlen(out_buf), 0);
      }
      else
      {
        printf("File %s is being sent \n", &file_name[1]);
        if ((strstr(file_name, ".jpg") != NULL)||(strstr(file_name, ".gif") != NULL)) 
        {
          strcpy(out_buf, OK_IMAGE);
        }
        else
        {
          strcpy(out_buf, OK_TEXT);
        }

        send(acceptfd, out_buf, strlen(out_buf), 0);
 
        buffer_length = 1;  
        while (buffer_length > 0)  
        {
          buffer_length = read(fd1, out_buf, BUF_SIZE);
          if (buffer_length > 0)   
          { 
            send(acceptfd, out_buf, buffer_length, 0);     
                 
/* end of code adapted from http://kturley.com/simple-multi-threaded-web-server-written-in-c-using-pthreads/ */
    
  printf("\nin serving thread after sending file\n");
    
    //pthread_mutex_lock(&sthread_mutex);
    
            sem_post(sem);
    printf("\n got mutex ,incremented number of free threads: %d\n",free_thread);
    //pthread_mutex_unlock(&sthread_mutex);
    printf("\nafter semaphore post\n");
          }
        }
      }
    }
  }
}

//scheduler thread
void *thread_scheduler(void *arg)
{
  unsigned int schedalg=*((unsigned int*)arg);
  int acceptfd,n;
  if(schedalg==0)
  {  
    while(1)
    {  
      if(front!=NULL)
      {  
        sem_wait(sem);    
        //printf("\nin sched thread before extracting element\n");
        //printf("\nscheduler locking mutex\n");    
        pthread_mutex_lock(&sthread_mutex);
        pthread_mutex_lock(&qmutex);
        r2=extract_element();
        //printf("\n popped element in scheduler thread");
        pthread_mutex_unlock(&qmutex);
        
        //printf("\nscheduler unlocked mutex\n");
        // call serving thread from thread pool
        
        //printf("\nin sched thread before sending to serving thread\n");
        
        pthread_cond_signal(&cond_var);
        
        
        free_thread--;
        pthread_mutex_unlock(&sthread_mutex);        
        //printf("\nin sched thread unlocked sthread mutex\n");
        
        //thread_serve(&r2);
        //pthread_create(&t_serve,NULL,&thread_serve,&r);  
        //thread_serve(&r);
      }
      else 
      {
        continue;
      }      
    }
  }
  else
  {
  //code for SJF scheduling algorithm
    //printf("\n entered SJF scheduling algorithm");
    int shortestjob_fd=0;
    int min;
    int a,b;
    while(1)      
    {  pthread_mutex_lock(&qmutex);
      temp=front;
      if (temp==NULL)
      {
        continue;
      }
      else if(temp->link==NULL) 
      {
        //printf("\n only one");
        shortestjob_fd=temp->r.acceptfd;
      }
      else
      {
        min=temp->r.size;
        while(temp->link!=NULL)  //should modify
        {
          b=temp->link->r.size;
          if(min<=b)
          {
            shortestjob_fd=temp->r.acceptfd;
          }    
          else if(min>b)
          {      
            min=temp->link->r.size;            
            shortestjob_fd=temp->link->r.acceptfd;
          }
          printf("\n %d",a);
          temp=temp->link;
        }
      }
      pthread_mutex_lock(&sthread_mutex);
      
      r2=removesjf(shortestjob_fd);
      //printf("extracted element");
      pthread_cond_signal(&cond_var);
      pthread_mutex_unlock(&sthread_mutex);
            
      pthread_mutex_unlock(&qmutex);
    }
  }
}
