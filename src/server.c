#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include "../include/protocol.h"
#include "../include/util.h"
#include "../include/cmd_handler.h"
#include "../include/server.h"
#include "../include/errors.h"

int   get_server_sock(char* service);
int   accept_client(int server_fd);
void* client_th(void* th_args);
void  handle_tcp_client(int client_fd);
int   create_thread_safe(int client_fd);
int   send_resp_to_clnt(void* buf,
                        size_t size,
                        int client_fd,
                        unsigned int n_retries,
                        unsigned int retry_tm);
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
    int client_fd = ((ClientArgs*)th_args)->client_fd;
    free(th_args);
    handle_tcp_client(client_fd);
    return NULL;
}
void
handle_tcp_client(int client_fd){
    int  rtn_v;
    char buff, *rtn_s;
    ssize_t rtn;
    while(1){
        rtn = recv(client_fd,&buff,1,0);
        if(rtn==-1){
            printf("Invalid data received: %s...\n", strerror(errno));
		    break;
        }
        if(!rtn){
            break;
        }
        switch (buff){
            case '*':
                ArrayNode* array = parse_array(client_fd);
                if(array){
                    print_array(array,0);
                    rtn_s = parse_command(array);
                }
                if(array && rtn_s){
                    rtn_v = send(client_fd,rtn_s,strlen(rtn_s),0);
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
    close(client_fd);
    pthread_mutex_lock(&nbr_th_mutex);
    nbr_th--;
    pthread_mutex_unlock(&nbr_th_mutex);
}
int
create_thread_safe(int client_fd){
    ClientArgs* args = (ClientArgs*) calloc(1,sizeof(ClientArgs));
    pthread_t   thid;
    char* err_msg = "Error: connection refused.";
    if(!args){
        return -1;
    }
    args->client_fd = client_fd;
    pthread_mutex_lock(&nbr_th_mutex);
    if(nbr_th>MAX_THREAD_N){
        pthread_mutex_unlock(&nbr_th_mutex);
        free(args);
        send_resp_to_clnt(err_msg,strlen(err_msg)+1, client_fd,3,500);
        //TODO: check For some reason, when the client_fd is closed, the socket is resending the message
        close(client_fd);
        return -1;
    }
    int err = pthread_create(&thid, NULL, client_th, args);
    if(err==0){
        nbr_th++;
        pthread_mutex_unlock(&nbr_th_mutex);
        return 1;
    }
    pthread_mutex_unlock(&nbr_th_mutex);
    log_pthread_create_err(err);
    send_resp_to_clnt(err_msg,strlen(err_msg)+1, client_fd,3,500);
    close(client_fd);
    return -1;
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
int
send_resp_to_clnt(void* buf,
                  size_t size,
                  int client_fd,
                  unsigned int n_retries,
                  unsigned int retry_tm){
    ssize_t rtn;
    rtn = send(client_fd,buf,size,MSG_WAITALL);
    if(rtn!=-1){
        return (int)rtn;
    }
    perror("Error: ");
    SendErrorInfo* err_info;
    for(size_t i = 0; i < n_retries; i++){
        err_info = categorize_send_error(errno);
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
