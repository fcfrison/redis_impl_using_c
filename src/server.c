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
#include "../include/protocol.h"
#include "../include/util.h"
#include "../include/cmd_handler.h"
#include "../include/server.h"
// https://tutorial.codeswithpankaj.com/c-programming/thread

int   get_server_sock(char* service);
int   accept_client(int server_fd);
void* client_th(void* th_args);
void  handle_tcp_client(int client_fd);
int   create_thread_safe(int client_fd);
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
    if(!args){
        return -1;
    }
    args->client_fd = client_fd;
    pthread_mutex_lock(&nbr_th_mutex);
    if(nbr_th>MAX_THREAD_N){
        free(args);
        // TODO: improve error message handling
        char* msg = "Error: connection refused.";
        send(client_fd,msg,sizeof(msg),MSG_WAITALL);
        close(client_fd);
        pthread_mutex_unlock(&nbr_th_mutex);
        return -1;
    }
    int err = pthread_create(&thid, NULL, client_th, args);
    if(err==0){
        nbr_th++;
        pthread_mutex_unlock(&nbr_th_mutex);
        return 1;
    }
    if(err!=0) {
        switch(err) {
            case EAGAIN:
                log_error("System lacks resources to create thread");
                break;
            case EINVAL:
                log_error("Invalid thread attributes.");
                break;
            case ENOMEM:
                log_error("Insufficient memory to create thread");
                break;
            default:
                log_error("Unknown error creating thread");
        }
    }
    char* msg = "Error: connection refused.";
    send(client_fd,msg,sizeof(msg),MSG_WAITALL);
    close(client_fd);
    pthread_mutex_unlock(&nbr_th_mutex);
    return -1;
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
