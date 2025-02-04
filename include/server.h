#ifndef SERVER_H
#define BUFSIZE       ((size_t)10)
#define MAXPENDING    10
#define MAX_THREAD_N  2
#define MAX_QUEUE_TIME 5
#define MAX_APP_WORKERS 5
#include "queue.h"
#include <semaphore.h>
typedef struct Request Request;
typedef struct ClientArgs ClientArgs; 
typedef struct ThReqQueueMngrArgs ThReqQueueMngrArgs;
typedef struct ThreadFunc ThreadFunc; 
typedef struct ThExcdTmQMgr ThExcdTmQMgr; 
typedef struct AppWorker AppWorker;
struct Request{
    int fd;
    struct timespec* ts;
};

struct ThReqQueueMngrArgs{
    Queue*           req_q;
    pthread_mutex_t* req_q_mtx;
    Queue*           excd_tm_q;
    pthread_mutex_t* excd_tm_mtx;
    sem_t*           excd_tm_sem;
};
struct ThreadFunc{
    void* (*fptr)(void*);
    void* args;
};
struct ThExcdTmQMgr{
    Queue*           excd_tm_q;
    pthread_mutex_t* excd_tm_mtx;
    sem_t*           excd_tm_sem;
};
struct AppWorker{
    Queue*           req_q;
    pthread_mutex_t* req_q_mtx;
    sem_t*           req_q_sem;
};
void
get_time_diff(struct timespec* a, 
              struct timespec* b,
              struct timespec* diff);
char
was_waiting_time_exceeded(struct timespec* diff,
                 float  dlt_sec);

int read_exact_bytes(int fd, char* buf, size_t len);
#endif