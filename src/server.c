#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <sys/time.h>
#include <semaphore.h>
#include "../include/protocol.h"
#include "../include/util.h"
#include "../include/cmd_handler.h"
#include "../include/server.h"
#include "../include/errors.h"
#include "../include/queue.h"

int   get_server_sock(char* service);
int   accept_client(int server_fd);
void* client_th(void* th_args);
void* app_code(int fd);
int   create_thread_safe(int client_fd);
ClientArgs* init_client(void);
ssize_t send_resp_to_clnt(void* buf,
                          size_t size,
                          int client_fd,
                          unsigned int n_retries,
                          unsigned int retry_tm);
ssize_t recv_resp_fm_clnt(void* buf,
                          size_t size,
                          int fd,
                          unsigned int n_retries,
                          unsigned int retry_tm);
int  read_exact_bytes(int fd, char* buf, size_t len);
void mutex_unlock(pthread_mutex_t* mtx);
void mutex_lock(pthread_mutex_t* mtx,
           unsigned int     n_retries,
           unsigned int     retry_tm);
ThreadFunc* init_th_req_queue_mgr(Queue*           req_q,
                                  pthread_mutex_t* req_q_mtx,
                                  Queue*           excd_tm_q,
                                  pthread_mutex_t* excd_tm_mtx,
                                  sem_t*           excd_tm_sem);
ThreadFunc* init_th_excd_tm_q_mgr(Queue*           excd_tm_q,
                                  pthread_mutex_t* excd_tm_mtx,
                                  sem_t*           excd_tm_sem);
void* th_req_queue_mgr(void* args);
void* th_excd_tm_q_mgr(void* args);
char was_waiting_time_exceeded(struct timespec* diff,
                               float  dlt_sec);
void get_time_diff(struct timespec* a, 
                   struct timespec* b,
                   struct timespec* diff);
ThreadFunc* init_th_req_queue_mgr(Queue*           req_q,
                                   pthread_mutex_t* req_q_mtx,
                                   Queue*           excd_tm_q,
                                   pthread_mutex_t* excd_tm_mtx,
                                   sem_t*           excd_tm_sem);
Request* init_request(int fd);
void* run_thread(void* _args);
void create_thread(void* args);
// global vars
unsigned int    nbr_th = 0;
pthread_mutex_t nbr_th_mutex;

int
get_server_sock(char* service) {
    struct addrinfo addr_config;
    memset(&addr_config, 0, sizeof(addr_config));
    addr_config.ai_family   = AF_INET6;
    addr_config.ai_flags    = AI_PASSIVE;
    addr_config.ai_socktype = SOCK_STREAM;
    addr_config.ai_protocol = IPPROTO_TCP;

    struct addrinfo* addr_list;
    int rtn_value = getaddrinfo(NULL, service, &addr_config, &addr_list);
    if (rtn_value != 0) {
        return -1;
    }

    int fd = -1;
    for (struct addrinfo* l_item = addr_list; l_item != NULL; l_item = l_item->ai_next) {
        fd = socket(l_item->ai_family, l_item->ai_socktype, l_item->ai_protocol);
        if (fd < 0) {
            continue;
        }

        int bind_result = bind(fd, l_item->ai_addr, l_item->ai_addrlen);
        int listen_result = listen(fd, MAXPENDING);
        if ((bind_result == 0) && (listen_result == 0)) {
            int val = 1;
            setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)); // Enable the option
            break;
        }
    }

    return fd;
}
int
accept_client(int server_fd){
    struct sockaddr_in client_addr;
    socklen_t client_addr_len;
    client_addr_len = sizeof(client_addr);
    return accept(server_fd, (struct sockaddr*)&client_addr, &client_addr_len);
}

void*
client_th(void* th_args){
    pthread_t th_id = pthread_self();
    // Guarantees that thread resources are deallocated upon return
    pthread_detach(th_id);
    int fd = ((ClientArgs*)th_args)->fd;
    void* (*fnc)(int);
    fnc = ((ClientArgs*)th_args)->fptr;
    free(th_args);
    fnc(fd);
    return NULL;
}
void*
app_code(int fd){
    int  rtn_v;
    char buff, *rtn_s;
    ssize_t rtn;
    while(1){
        rtn = recv_resp_fm_clnt(&buff, 1, fd, 3, .500);
        if(rtn==-1){
            printf("Invalid data received: %s...\n", strerror(errno));
		    break;
        }
        if(!rtn){
            break;
        }
        switch (buff){
            case '*':
                ArrayNode* array = parse_array(fd);
                if(array){
                    print_array(array,0);
                    rtn_s = parse_command(array);
                }
                if(array && rtn_s){
                    rtn_v = send_resp_to_clnt(rtn_s,strlen(rtn_s)+1, fd,3,500);
                    if(rtn_v==-1){
                        continue;
                    }
                    free(rtn_s);
                    delete_array(array,1);
                }
                break;
            
            default:
                break;
        }

	}
    close(fd);
    mutex_lock(&nbr_th_mutex,3,1000);
    nbr_th--;
    mutex_unlock(&nbr_th_mutex);
    return NULL;
}
int
create_thread_safe(int fd){
    char* err_msg = "Error: connection refused.";
    if(nbr_th>MAX_THREAD_N){
        pthread_mutex_unlock(&nbr_th_mutex);
        send_resp_to_clnt(err_msg,strlen(err_msg)+1, fd,3,500);
        close(fd);
        return -1;
    }
    ClientArgs* args = init_client();
    if(!args){
        return -1;
    }
    pthread_t thid;
    args->fd = fd;
    mutex_lock(&nbr_th_mutex,3,1000);
    int err = pthread_create(&thid, NULL, client_th, args);
    if(err==0){
        nbr_th++;
        mutex_unlock(&nbr_th_mutex);
        return 1;
    }
    mutex_unlock(&nbr_th_mutex);
    log_pthread_create_err(err);
    send_resp_to_clnt(err_msg,strlen(err_msg)+1, fd,3,500);
    close(fd);
    return -1;
}

// I didn't figure it out yet how to make the server completely app agnostic
// Probably some routine created by the user
ClientArgs*
init_client(void){
    ClientArgs* args = (ClientArgs*) calloc(1,sizeof(ClientArgs));
    if(!args){
        return NULL;
    }
    args->fd   = -1;
    args->fptr = &app_code;
    return args;
}
/**
* Sends data to a client socket with retry capability.
*
* @param buf        Pointer to data buffer to send
* @param size       Size of data in bytes
* @param client_fd  Client socket file descriptor
* @param n_retries  Maximum number of retry attempts on failure
* @param retry_tm   Time to wait between retries in milliseconds
*
* @return  Number of bytes sent on success
*         -1 on error after all retries exhausted
*
* Uses MSG_WAITALL flag to ensure complete transmission. On error,
* categorizes the error type and retries non-fatal errors after
* waiting retry_tm milliseconds, up to n_retries times.
*/
ssize_t
send_resp_to_clnt(void* buf,
                  size_t size,
                  int client_fd,
                  unsigned int n_retries,
                  unsigned int retry_tm){
    ssize_t rtn;
    rtn = send(client_fd,buf,size,MSG_WAITALL);
    if(rtn!=-1){
        return rtn;
    }
    perror("Error: ");
    SendErrorInfo* err_info = NULL;
    for(size_t i = 0; i < n_retries; i++){
        if(err_info){
            free(err_info);
        }
        err_info = categorize_send_error(errno);
        if(!err_info){
            return rtn;
        }
        if((err_info->category==ERROR_FATAL) || (msleep(retry_tm,3)==-1)){
            free(err_info);
            return rtn;
        }
        rtn = send(client_fd,buf,size,MSG_WAITALL);
        if(rtn!=-1){
            break;
            }
        perror("Error: ");
    }
    if(err_info){
        free(err_info);
    }
    return rtn;
    
}
ssize_t
recv_resp_fm_clnt(void* buf,
                  size_t size,
                  int fd,
                  unsigned int n_retries,
                  unsigned int retry_tm){
    ssize_t rtn;
    rtn = recv(fd, buf, size, MSG_WAITALL);
    if(rtn!=-1){
        return rtn;
    }
    perror("Error: ");
    RecvErrorInfo* err_info = NULL;
    for(size_t i = 0; i < n_retries; i++){
        if(err_info){
            free(err_info);
        }
        err_info = categorize_recv_error(errno);
        if(!err_info){
            return rtn;
        }
        if((err_info->category==ERROR_FATAL) || (msleep(retry_tm,3)==-1)){
            free(err_info);
            return rtn;
        }
        rtn = recv(fd, buf, size, MSG_WAITALL);
        if(rtn!=-1){
            break;
            }
        perror("Error: ");
    }
    if(err_info){
        free(err_info);
    }
    return rtn;
};

int 
read_exact_bytes(int fd, char* buf, size_t len){
    ssize_t rtn;
    rtn = recv_resp_fm_clnt(buf, len, fd, 3, 500);
    if(!rtn){
        log_error("connection closed by the client");
        return 0;
    }
    if((size_t)rtn < len){
        log_error("less bytes than the expected were received");
        return 0;
    }
    return 1;
    };

/**
 * Attempts to lock a mutex with retry and error handling capabilities.
 *
 * @param mtx Pointer to the pthread mutex to be locked
 * @param n_retries Maximum number of retry attempts if initial lock fails
 * @param retry_tm Time (in milliseconds) to wait between retry attempts
 *
 * @error_handling
 * - Prints error details using perror()
 * - Exits the program if:
 *   1. Error categorization fails
 *   2. A fatal error is encountered
 *   3. Sleep between retries fails
 *   4. Maximum retry attempts are exhausted
 * @warning Terminates the entire program on persistent lock failures
 */
void
mutex_lock(pthread_mutex_t* mtx,
           unsigned int     n_retries,
           unsigned int     retry_tm){
    int rtn;
    rtn = pthread_mutex_lock(mtx);
    if(rtn==0){
        return;
    }
    perror("Error: ");
    PthreadMutexLockErrorInfo* err_info = NULL;
    for(size_t i = 0; i < n_retries; i++){
        if(err_info){
            free(err_info);
        }
        err_info = categorize_mtx_lck_error(rtn);
        if(!err_info){
            exit(EXIT_FAILURE);
        }
        if((err_info->category==ERROR_FATAL) || (msleep(retry_tm,3)==-1)){
            free(err_info);
            exit(EXIT_FAILURE);
        }
        rtn = pthread_mutex_lock(mtx);
        if(rtn==0){
            free(err_info);
            return;
            }
        perror("Error: ");
    }
    if(err_info){
        free(err_info);
    }
    exit(EXIT_FAILURE);
};

void 
mutex_unlock(pthread_mutex_t* mtx){
    int rtn;
    rtn = pthread_mutex_unlock(mtx);
    if(rtn==0){
        return;
    }
    exit(EXIT_FAILURE);
};

ThreadFunc*
init_th_req_queue_mgr(
    Queue*           req_q,
    pthread_mutex_t* req_q_mtx,
    Queue*           excd_tm_q,
    pthread_mutex_t* excd_tm_mtx,
    sem_t*           excd_tm_sem){
    
    ThreadFunc* th_func = (ThreadFunc*)calloc(1,sizeof(ThreadFunc));
    ThReqQueueMngrArgs* th_func_args = (ThReqQueueMngrArgs*) calloc(1,sizeof(ThReqQueueMngrArgs));
    th_func_args->req_q       = req_q;
    th_func_args->req_q_mtx   = req_q_mtx;
    th_func_args->excd_tm_q   = excd_tm_q;
    th_func_args->excd_tm_mtx = excd_tm_mtx;
    th_func_args->excd_tm_sem = excd_tm_sem;
    th_func->fptr             = &th_req_queue_mgr;
    th_func->args             = (void*) th_func_args;
    return th_func;
    }

ThreadFunc*
init_th_excd_tm_q_mgr(Queue*           excd_tm_q,
                      pthread_mutex_t* excd_tm_mtx,
                      sem_t*           excd_tm_sem){
    ThreadFunc*   th_func = (ThreadFunc*)calloc(1,sizeof(ThreadFunc));
    ThExcdTmQMgr* th_func_args = (ThExcdTmQMgr*)calloc(1,sizeof(ThExcdTmQMgr));
    th_func_args->excd_tm_q   = excd_tm_q;
    th_func_args->excd_tm_mtx = excd_tm_mtx;
    th_func_args->excd_tm_sem = excd_tm_sem;
    th_func->fptr             = &th_excd_tm_q_mgr;
    th_func->args             = (void*)th_func_args;
    return th_func;
}
void*
th_req_queue_mgr(void* args){
    ThReqQueueMngrArgs* _args     = (ThReqQueueMngrArgs*) args;
    Queue*           req_q        = _args->req_q;
    pthread_mutex_t* req_q_mtx    = _args->req_q_mtx;  
    Queue*           excd_tm_q    = _args->excd_tm_q; 
    pthread_mutex_t* excd_tm_mtx  = _args->excd_tm_mtx;
    sem_t*           excd_tm_sem  = _args->excd_tm_sem;
    free(args);
    if(!req_q){
        return NULL;
    }
    Request*        request;
    struct timespec curr_tm, diff;
    float           delta = 1.0;
    //curr_time
    while(1){
        msleep(1000,3);
        mutex_lock(req_q_mtx,3,1000);
        clock_gettime(CLOCK_MONOTONIC, &curr_tm);
        request = req_q->get_front(req_q);
        do{
            if(!request){
                break;
            }
            get_time_diff(&curr_tm, request->ts, &diff);
            if(!was_waiting_time_exceeded(&diff,delta)){
                break;
            }
            // waiting time exceeded
            req_q->dequeue(req_q);
            // insert "front" Request in the denied connection queue
            mutex_lock(excd_tm_mtx,3,1000);
            excd_tm_q->enqueue(excd_tm_q,request);
            // TODO: create routines for dealing with errors in pthread_mutex_unlock
            mutex_unlock(excd_tm_mtx);
            // signal th_excd_tm_q_mgr
            sem_post(excd_tm_sem);
            request = req_q->get_front(req_q);
        } while(request);
        pthread_mutex_unlock(req_q_mtx);
        
    }
    return NULL;
};

void*
th_excd_tm_q_mgr(void* args){
    ThExcdTmQMgr* _args =  (ThExcdTmQMgr*) args;
    Queue*           excd_tm_q = _args->excd_tm_q;
    pthread_mutex_t* excd_tm_mtx = _args->excd_tm_mtx;
    sem_t*           excd_tm_sem = _args->excd_tm_sem;
    free(args);
    Request* rq;
    char* msg = "Timeout Error: the connection couldn't be established";
    while(1){
        //TODO: create routine to handle the sem_wait sycall
        sem_wait(excd_tm_sem);
        mutex_lock(excd_tm_mtx,3,1000);
        rq = excd_tm_q->dequeue(excd_tm_q);
        pthread_mutex_unlock(excd_tm_mtx);
        if(!rq){
            continue;
        }
        send_resp_to_clnt(msg,strlen(msg)+1,rq->fd,3,1000);
        close(rq->fd);
        free(rq->ts);
        free(rq);         
    }
    return NULL;
}

char
was_waiting_time_exceeded(struct timespec* diff,
                          float  dlt_sec){
    if(dlt_sec>MAX_QUEUE_TIME){
        dlt_sec=(float)MAX_QUEUE_TIME;
    }
    float diff_t = diff->tv_sec + diff->tv_nsec/1e9;
    if(diff_t>dlt_sec) return 1;
    return 0;
}

void
get_time_diff(struct timespec* a, 
              struct timespec* b,
              struct timespec* diff){
        diff->tv_sec  = a->tv_sec - b->tv_sec;
        diff->tv_nsec = a->tv_nsec - b->tv_nsec;
        return;
    }
Request*
init_request(int fd){
    Request* rq = (Request*)calloc(1,sizeof(Request));
    struct timespec* ts = (struct timespec*)calloc(1,sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC, ts);
    rq->fd = fd;
    rq->ts = ts;
    return rq;
}
void*
run_thread(void* _args){
    ThreadFunc* args = (ThreadFunc*)_args;
    pthread_t th_id = pthread_self();
    // Guarantees that thread resources are deallocated upon return
    pthread_detach(th_id);
    void* fn_args = args->args;
    void* (*fnc)(void*);
    fnc = args->fptr;
    free(args);
    fnc(fn_args);
    return NULL;
}
void create_thread(void* args){
    pthread_t thid;
    int err = pthread_create(&thid, NULL, run_thread, args);
    if(err==0){
        return;
    }
    exit(EXIT_FAILURE);
}
int
main() {
    pthread_mutex_t*    excd_tm_mtx = (pthread_mutex_t*)calloc(1,sizeof(pthread_mutex_t));
    pthread_mutex_t*    req_q_mtx   = (pthread_mutex_t*)calloc(1,sizeof(pthread_mutex_t));
    Queue*              excd_tm_q   = init_queue(NULL);
    Queue*              req_q       = init_queue(NULL);
    pthread_mutexattr_t attr;
    sem_t*              excd_tm_sem = (sem_t*)calloc(1,sizeof(sem_t));
	int server_fd, client_fd;
    // initialize mutexes
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setrobust(&attr, PTHREAD_MUTEX_ROBUST);
    pthread_mutex_init(req_q_mtx, &attr);
    pthread_mutex_init(excd_tm_mtx, &attr);
    pthread_mutexattr_destroy(&attr);
    // initialize semaphores
    sem_init(excd_tm_sem, 0, 0);
    // initialize th_req_queue_mgr
    ThreadFunc* th_rq_q_mng =  init_th_req_queue_mgr(req_q, req_q_mtx, excd_tm_q, excd_tm_mtx,excd_tm_sem);
    create_thread((void*)th_rq_q_mng);
    // initialize th_excd_tm_q_mgr
    ThreadFunc* th_exd_tm_mgr = init_th_excd_tm_q_mgr(excd_tm_q,excd_tm_mtx,excd_tm_sem);
    create_thread((void*)th_exd_tm_mgr);
	server_fd = get_server_sock("6379");
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return -1;
	}
    while(1){
	    printf("Waiting for a client to connect...\n");
        client_fd = accept_client(server_fd);
        if(client_fd==-1) continue;
        //create_thread_safe(client_fd);
        Request* rq = init_request(client_fd);
        pthread_mutex_lock(req_q_mtx);
        req_q->enqueue(req_q,rq);
        pthread_mutex_unlock(req_q_mtx);
    }
	close(server_fd);
    // TODO: the clean up routines are bellow. Probably,
    // there's a better approach out there (for instance, registering the clean up routine
    // when using exit(). Read more about it.).
    // Calling pthread_mutex_destroy this way is prone to errors. Again, probably there is 
    // a better approach.
    pthread_mutex_destroy(req_q_mtx);
    pthread_mutex_destroy(excd_tm_mtx);
    sem_destroy(excd_tm_sem);
    free(th_rq_q_mng->args);
    free(th_rq_q_mng);
    free(th_exd_tm_mgr->args);
    free(th_exd_tm_mgr);
    free(req_q);
    free(excd_tm_q);
    
	return 0;
}
