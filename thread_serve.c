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

#include <assert.h>

#include "main.h"

struct request pop_message();

struct request extract_element()
{
  if(front_node==NULL)
    log_msg("empty queue");
  else
  {
    struct request r1;

    struct node *p_node = front_node;
    printf("extracted element: %d",p_node->r.acceptfd);
    front_node=front_node->link;
    r1.acceptfd=p_node->r.acceptfd;
    strcpy(r1.file_name,p_node->r.file_name);
    r1.size=p_node->r.size;
    free(p_node);
    return(r1);
  }

  struct request result;
  return result;
}

void *thread_serve()
{
  while(1)
  {
    struct request r = pop_message();

    time_t now;
    time(&now);
    struct tm * ct = localtime(&now);
    char ch[128], time_serve[128];
    struct timeval tv;
    strftime(ch, sizeof ch, "[%Y-%m-%d %H:%M:%S%z]", ct);
    snprintf(time_serve, sizeof time_serve, ch, tv.tv_usec);

    unsigned int ip=r.cli_ipaddr;

    unsigned char bytes[4];
    bytes[0] = ip & 0xFF;
    bytes[1] = (ip >> 8) & 0xFF;
    bytes[2] = (ip >> 16) & 0xFF;
    bytes[3] = (ip >> 24) & 0xFF;

    if (debug_flag == 0 && log_flag == 1)
    {
      FILE * file_des=fopen(file,"a");
      log_msg("in serving thread");

      fprintf(file_des,"%d.%d.%d.%d\t-\t ", bytes[0], bytes[1], bytes[2], bytes[3]);
      fprintf(file_des,"%s\t %s\t %s \t status\t %d\n",r.time_arrival,time_serve,r.in_buf,r.size);

      fclose(file_des);
    }
    else if (debug_flag == 1)
    {
      printf("%d.%d.%d.%d\t-\t %s\t %s\t %s \t status\t %d\n", bytes[0], bytes[1], bytes[2], bytes[3],r.time_arrival,time_serve,r.in_buf,r.size);
      //fprintf(stdout,"%s\t %s\t %s \t status\t %d\n",r.time_arrival,time_serve,r.in_buf,r.size);
    }

    log_msg("in serving thread copied structure");

    char           in_buf[BUF_SIZE];
    char           out_buf[BUF_SIZE];
    char           *file_name;
    file_name = malloc(sizeof(char *));
    int acceptfd;
    unsigned int   fd1;
    unsigned int   buffer_length;
    unsigned int   retcode;
    int m;

    log_msg("in serving thread before copying variables");
    acceptfd = r.acceptfd;
    file_name = r.file_name;
    log_msg("in serving thread after copying variables");

    printf("in serving thread file name is %s\n",file_name);

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
      close(acceptfd);
    }
  }
}

struct request pop_message() {
  struct request result;

  log_msg("sssExtracting next item from list...");
  pthread_mutex_lock(&qmutex);

  while (front_node == NULL) {
    log_msg("checking...");

    //It is my understanding that pthread_cond_wait will wait until notified on the cond_var, but I had some issues around this.
    int res = pthread_cond_wait(&cond_var, &qmutex);
    if (res != 0)
      printf("**************************************************** pthread_cond_wait error: %d\n", res);
  }

  // hmmmm do I want this or something else?
  assert(front_node != NULL);
  result = extract_element();

  pthread_mutex_unlock(&qmutex);
  log_msg("sssDone Extracting next item from list.");

  return result;

}
