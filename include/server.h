#ifndef SERVER_H
#define BUFSIZE       ((size_t)10)
#define MAXPENDING    10
#define MAX_THREAD_N  2
#define MAX_QUEUE_TIME 5
typedef struct Request Request;
typedef struct ClientArgs ClientArgs; 

struct Request{
    int fd;
    struct timespec* ts;
};

struct ClientArgs {
    int   fd;
    void* (*fptr)(int);
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