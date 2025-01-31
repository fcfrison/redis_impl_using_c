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
#include "../include/protocol.h"
#include "../include/util.h"
#include "../include/cmd_handler.h"
#include "../include/server.h"
#include "../include/errors.h"

int   get_server_sock(char* service);
int   accept_client(int server_fd);
void* client_th(void* th_args);
void* app_code(int fd);
int   create_thread_safe(int client_fd);
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
ClientArgs* init_client(void);

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
    pthread_mutex_lock(&nbr_th_mutex);
    nbr_th--;
    pthread_mutex_unlock(&nbr_th_mutex);
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
    pthread_mutex_lock(&nbr_th_mutex);
    int err = pthread_create(&thid, NULL, client_th, args);
    if(err==0){
        nbr_th++;
        pthread_mutex_unlock(&nbr_th_mutex);
        return 1;
    }
    pthread_mutex_unlock(&nbr_th_mutex);
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
        if((err_info->category==SEND_ERROR_FATAL) || (msleep(retry_tm,3)==-1)){
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
    RecvErrorInfo* err_info;
    for(size_t i = 0; i < n_retries; i++){
        if(err_info){
            free(err_info);
        }
        err_info = categorize_recv_error(errno);
        if(!err_info){
            return rtn;
        }
        if((err_info->category==RECV_ERROR_FATAL) || (msleep(retry_tm,3)==-1)){
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
}
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


int
main() {
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
    
	int server_fd, client_fd;
	server_fd = get_server_sock("6379");
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return -1;
	}
    while(1){
	    printf("Waiting for a client to connect...\n");
        client_fd = accept_client(server_fd);
        if(client_fd==-1) continue;
        create_thread_safe(client_fd);
    }
	close(server_fd);
	return 0;
}
