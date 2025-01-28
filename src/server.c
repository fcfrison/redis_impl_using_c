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
#define BUFSIZE ((size_t)10)
#define MAXPENDING 10
// https://tutorial.codeswithpankaj.com/c-programming/thread

typedef ClientArgs ClientArgs; 
struct ClientArgs {
    int client_fd;
};

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
    int  client_fd, rtn_v;
    char buff, *rtn_s;
    ssize_t rtn;
    while(1){
        rtn = recv(client_fd,&buff,1,0);
        if(rtn==-1){
            printf("Invalid data received: %s...\n", strerror(errno));
		    return -1;
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
}

int
main() {
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	int server_fd, client_fd, rtn_v;
	char buff, *rtn_s;
    ssize_t rtn;
	server_fd = get_server_sock("6379");
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return -1;
	}
    while(1){
	    printf("Waiting for a client to connect...\n");
        client_fd = accept_client(server_fd);
        if(client_fd==-1) continue;
        ClientArgs* args = (ClientArgs*) calloc(1,sizeof(ClientArgs));
        if(!args) continue;
        args->client_fd = client_fd;
        //TODO: implement pthread_create code
        pthread_create(NULL, NULL, NULL, NULL);

    }
	close(server_fd);
	return 0;
}
