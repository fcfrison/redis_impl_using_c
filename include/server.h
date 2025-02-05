#ifndef SERVER_H
#define MAX_QUEUE_TIME  5
#define MAX_APP_WORKERS 100
#define RECV_TIMEOUT    10
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
    int              max_queue_time;
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
    unsigned int     id;
    Queue*           req_q;
    pthread_mutex_t* req_q_mtx;
    sem_t*           req_q_sem;
    void*            (*func)(void*);
    time_t           recv_timeout;
};

void 
start_server(void*  (*fptr)(void*),
             unsigned int    maxpending,
             unsigned int    max_queue_time,
             unsigned int    max_app_workers,
             time_t          recv_timeout);

ssize_t recv_resp_fm_clnt(void*        buf,
                          size_t       size,
                          int          fd,
                          unsigned int n_retries,
                          unsigned int retry_tm);

ssize_t send_resp_to_clnt(void* buf,
                          size_t size,
                          int client_fd,
                          unsigned int n_retries,
                          unsigned int retry_tm);
#endif