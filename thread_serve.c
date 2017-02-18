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

#include "main.h"

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
        printf("File %s is being sent\n", &file_name[1]);
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
          }
        }

        //Done sending to client

        //pthread_mutex_lock(&sthread_mutex);
        sem_post(sem);
        //pthread_mutex_unlock(&sthread_mutex);
        log_msg("after semaphore post");
      }
    }
  }
}

//scheduler thread
void *thread_scheduler(void *arg)
{
  int acceptfd,n;

  while(1)
  {
    if(front!=NULL)
    {
      log_msg("Going into sem_wait");

      //Wait for an available thread
      sem_wait(sem);
      log_msg("in sched thread before extracting element");
      log_msg("scheduler locking mutex");
      pthread_mutex_lock(&sthread_mutex);

      log_msg("Extracting next item from list...");
      pthread_mutex_lock(&qmutex);
      r2 = extract_element();
      pthread_mutex_unlock(&qmutex);
      log_msg("Done Extracting next item from list.");

      // call serving thread from thread pool

      log_msg("in sched thread before sending to serving thread");

      pthread_cond_signal(&cond_var);

      pthread_mutex_unlock(&sthread_mutex);
      log_msg("in sched thread unlocked sthread mutex");
    }
  }
}
