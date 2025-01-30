#ifndef SERVER_H
#define BUFSIZE       ((size_t)10)
#define MAXPENDING    10
#define MAX_THREAD_N  2

typedef struct ClientArgs ClientArgs; 
struct ClientArgs {
    int client_fd;
};
int read_exact_bytes(int fd, char* buf, size_t len);
#endif