#ifndef __SOME_GUARD
#define __SOME_GUARD

#include "thread_serve.h"

#define BUF_SIZE 32768
#define OK_IMAGE    "HTTP/1.0 200 OK\nContent-Type:image/gif\n\n"
#define OK_TEXT     "HTTP/1.0 200 OK\nContent-Type:text/html\n\n"
#define NOTOK_404   "HTTP/1.0 404 Not Found\nContent-Type:text/html\n\n"
#define MESS_404    "<html><body><h1>FILE NOT FOUND</h1></body></html>"

sem_t *sem;

extern int received_interrupt;

extern int debug_flag;
extern int log_flag;
extern char * file;
extern pthread_t t_serve;

extern pthread_mutex_t qmutex;
extern pthread_mutex_t sthread_mutex;
extern pthread_cond_t cond_var;

struct request
{
  int acceptfd;
  int size;
  char file_name[1024];
  unsigned int cli_ipaddr;
  char time_arrival[1024];
  char in_buf[2048];

}r2;
// queue function declarations;
void insertion(int,char*, int, unsigned int,char*,char*);
//void insertion(int,string, int);
struct request extract_element();
void display();
void print_help_options();
void log_msg(char * message);

//queue structre
struct node
{
  struct request r;
  struct node *link;
};
typedef struct node N;

extern N *rear;
extern N *front;
extern N *p;
extern N *temp;
extern N *new;

#endif
