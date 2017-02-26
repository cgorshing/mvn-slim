#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
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
#include <signal.h>

#include "main.h"
#include "thread_serve.h"

pthread_mutex_t qmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t repo_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

pthread_t t_listener;

int received_interrupt = 0;

int debug_flag = 0;
int log_flag = 0;
char * file = NULL;
pthread_t t_serve;

struct node *front_node = NULL;
struct node *rear = NULL;

struct repository *repo_front;
struct repository *repo_rear;

void build_repo_list() {
  pthread_mutex_lock(&repo_mutex);

  struct repository *new_repo = (struct repository*)malloc(sizeof(struct repository));
  strcpy(new_repo->name, "Simple Beginning Repository - Hardcoded");
  strcpy(new_repo->path, "/opt/wrench/simple-repo-hardcoded");
  new_repo->type = REPO_HOSTED;
  new_repo->link = NULL;

  //TODO I think I like having a prefix to limit first instead of word reading.
  // repo_front is better than front_repo
  // maybe head and tail is better than front rear
  if (front_node == NULL)
    repo_front = new_repo;
  else
    repo_rear->link = new_repo;
  repo_rear = new_repo;

  pthread_mutex_unlock(&repo_mutex);
}


void log_msg(const char * message, ...) {
  time_t now;
  time(&now);
  struct tm * ct=localtime(&now); //getting localtime
  char ch[128], time_serve[128];
  struct timeval tv;
  strftime(ch, sizeof ch, "[%Y-%m-%d %H:%M:%S%z]", ct);
  snprintf(time_serve, sizeof time_serve, ch, tv.tv_usec);

  //va_list valist;
  //va_start(valist, message);

  //char *buff_message = NULL;
  //int size_needed = vsnprintf(buff_message, 0, message, valist);

  //buff_message = malloc(size_needed + 1); // 1 for a null character
  //vsnprintf(buff_message, size_needed, message, valist);
  //printf("%s", buff_message);

  //free(buff_message);

  printf("%s %s\n", time_serve, message);
}

void handle_term(int signum) {
  received_interrupt = 1;

  pthread_cancel(t_listener);

  write(0, "Received interrupt signal!\n", 27);
}

void display()
{
  if (front_node == NULL)
    log_msg("empty queue");
  else
  {
    struct node *temp = front_node;
    while (temp != NULL)
    {
      printf("acceptfd is %d, file name is %s, ip addr is %u, request is %s,time is %s\n",
          temp->r.acceptfd,
          temp->r.file_name,
          temp->r.cli_ipaddr,
          temp->r.in_buf,
          temp->r.time_arrival);
      temp=temp->link;
    }
  }
}

// queue functions
void insertion(int afd, char *f, unsigned int ip, char * time_arrival, char * in_buf)
{
  pthread_mutex_lock(&qmutex);

  struct node *new_node = (N*)malloc(sizeof(N));
  char a[1024];
  char b[1024];
  char c[1024];
  strcpy(a,f);
  strcpy(b,time_arrival);
  strcpy(c,in_buf);
  new_node->r.acceptfd=afd;
  strcpy(new_node->r.file_name,a);
  new_node->r.cli_ipaddr=ip;
  strcpy(new_node->r.time_arrival,b);
  strcpy(new_node->r.in_buf,c);

  //new_node->r.file_name=a;
  new_node->link=NULL;
  if(front_node==NULL)
    front_node=new_node;
  else
    rear->link=new_node;
  rear=new_node;
  log_msg("inserted request into queue");
  display();

  pthread_cond_signal(&cond_var);
  pthread_mutex_unlock(&qmutex);
}
// end of queue functions


// Listening and queueing thread
void *thread_listen(void *arg)
{
  int sockfd=*((int*)arg);
  int acceptfd;
  socklen_t clilen;
  int newsockfd[10];
  struct sockaddr_in cli_addr;
  clilen = sizeof(cli_addr);

  int retcode;
  char in_buf[BUF_SIZE];

  listen(sockfd,5);
  log_msg("in listening thread before listen");

  while(1)
  {
    log_msg("in listening thread before accept");
    acceptfd = accept(sockfd,(struct sockaddr *) &cli_addr,&clilen);

    if (acceptfd < 0)
      perror("error in accepting");

    unsigned int ip=cli_addr.sin_addr.s_addr;
    time_t now;
    time(&now);
    struct tm * ct=localtime(&now); //getting localtime
    char ch[128], time_arrival[128];
    struct timeval tv;
    strftime(ch, sizeof ch, "[%d/%b/%Y : %H:%M:%S %z]", ct); //format of the timestamp string we need
    snprintf(time_arrival, sizeof time_arrival, ch, tv.tv_usec); //printing the needed timestamp string

    /*  FILE * file_des=fopen(file,"a");
    fprintf(file_des,"%s\n",time_arrival);
    fclose(file_des);
    */
    char *file_name = (char*)malloc(sizeof(char *));

    memset(in_buf, 0, sizeof(in_buf));
    retcode = recv(acceptfd, in_buf, BUF_SIZE, 0);

    log_msg("in listening thread before getting file name");

    if (retcode < 0)
    {
      log_msg("recv error detected ...");
    }
    else
    {
      strtok(in_buf, " ");
      file_name = strtok(NULL, " ");
    }

    if(file_name != NULL)
    {
      log_msg("in listening thread after accepting and before inserting into queue");

      insertion(acceptfd, file_name, ip, time_arrival, in_buf);

      log_msg("inserted into queue");
      printf("newsockfd in thread: %p\n", newsockfd);
    }
    else
    {
      log_msg("Had a problem with parsing out the file_name.");
    }
  }
}

int main(int argc, char *args[])
{
  pthread_t t_serve[10];
  int sockfd;
  int ids;
  char *dir = (char*)malloc(sizeof(char *));
  file = (char*)malloc(sizeof(char *));


  //********************
  //HANDLE SIGTERM
  //********************
  //struct sigaction action;
  //memset(&action, 0, sizeof(struct sigaction));
  //action.sa_handler = handle_term;
  //sigaction(SIGTERM, &action, NULL);
  //sigaction(SIGINT, &action, NULL);
  signal(SIGINT, handle_term);
  //********************
  //HANDLE SIGTERM
  //********************

  int portnum = 8080;
  int threadnum = 4;
  int sleep_time = 60;
  int i;
  int help_flag = 0;
  int dir_flag = 0;
  int time_flag;

  build_repo_list();

  // Parser code
  for (i = 0; i < argc; ++i)
  {
    if(strcmp(args[i],"-h")==0)
    {
      help_flag=1;
    }
    else if(strcmp(args[i],"-n")==0)
    {
      threadnum=atoi(args[i+1]);
    }
    else if(strcmp(args[i],"-d")==0)
    {
      debug_flag=1;
      threadnum=1;
    }
    else if(strcmp(args[i],"-l")==0)
    {
      log_flag=1;
      file=args[i+1];
    }
    else if(strcmp(args[i],"-p")==0)
    {
      portnum=atoi(args[i+1]);
    }
    else if(strcmp(args[i],"-r")==0)
    {
      dir_flag=1;
      dir=args[i+1];
    }
    else if(strcmp(args[i],"-t")==0)
    {
      time_flag=1;
      sleep_time=atoi(args[i+1]);
    }
  }

  //printf( "debug : %d, help: %d, log: %d, file name : %s port num : %d, dir : %d dir name: %s, time :%d ,thread num : %d\n",debug_flag,help_flag,log_flag,file,portnum,dir_flag,dir,sleep_time,threadnum);
  //Parser code ends

  if(help_flag==1)      // printing help options and exit if -h option is specified
  {
    print_help_options();
    exit(1);
  }
  else if(dir_flag==1)      //changing directory if -d option is specified
  {
    if(chdir(dir)<0)
      {
        perror("directory doesnt exist\n");
        exit(1);
      }
  }

  struct sockaddr_in serv_addr;
  log_msg("before socket creation");
  sockfd = socket(AF_INET, SOCK_STREAM,0);      //creation of socket
  int sock_opt = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sock_opt, sizeof(int));

  printf("after socket creation socket id is %d\n", sockfd);
  if (sockfd < 0)
    perror("error creating socket\n");
  bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port =htons(portnum);
  log_msg("before bind");
  if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
    perror("binding error\n");
    exit(1);
  }
  log_msg("after bind");

  for(int w = 0; w < threadnum; ++w) {
    pthread_create(&t_serve[w], NULL, &thread_serve, NULL);
  }

  ids=sockfd;
  printf("before creating thread sockfd is %d\n",ids);
  pthread_create(&t_listener, NULL, &thread_listen, &ids);

  log_msg("Waiting on listener thread");
  pthread_join(t_listener, NULL);
  log_msg("Done Waiting on listener thread");

  log_msg("after join in main");

  display();
  close(sockfd);
  return 0;
}


void print_help_options()
{
  printf("−d : Enter debugging mode. That is, do not daemonize, only accept one connection at a \ntime and enable logging to stdout. Without this option, the web server should run as a daemon process in the background. \n−h : Print a usage summary with all options and exit. \n−l file : Log all requests to the given file. See LOGGING for details.\n−p port : Listen on the given port. If not provided, myhttpd will listen on port 8080. \n−r dir : Set the root directory for the http server to dir. \n−t time : Set the queuing time to time seconds. The default should be 60 seconds. \n−n threadnum: Set number of threads waiting ready in the execution thread pool to threadnum. \nThe default should be 4 execution threads.");
}
