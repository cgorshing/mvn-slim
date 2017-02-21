#ifndef __SOME_GUARD
#define __SOME_GUARD

#include "thread_serve.h"

#define BUF_SIZE 32768
#define OK_IMAGE    "HTTP/1.0 200 OK\nContent-Type:image/gif\n\n"
#define OK_TEXT     "HTTP/1.0 200 OK\nContent-Type:text/html\n\n"
#define NOTOK_404   "HTTP/1.0 404 Not Found\nContent-Type:text/html\n\n"
#define MESS_404    "<html><body><h1>FILE NOT FOUND</h1></body></html>"

extern int received_interrupt;

extern int debug_flag;
extern int log_flag;
extern char * file;
extern pthread_t t_serve;

extern pthread_mutex_t qmutex;
extern pthread_mutex_t repo_mutex;
extern pthread_cond_t cond_var;

struct repository {
  char name[64];
  char url[2048];
  char path[2048];
  int type;
  struct repository *link;
};
#define REPO_PROXY 32768
#define REPO_HOSTED REPO_PROXY + 1

extern struct repository *repo_rear;
extern struct repository *repo_front;
extern struct repository *repo_p;

struct request
{
  int acceptfd;
  char file_name[1024];
  unsigned int cli_ipaddr;
  char time_arrival[1024];
  char in_buf[2048];
};

// queue function declarations;
void insertion(int, char*, unsigned int, char*, char*);
void display();
void print_help_options();
void log_msg(const char * message, ...);

//queue structre
struct node
{
  struct request r;
  struct node *link;
};
typedef struct node N;

extern N *front_node;

#endif
