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
#define BUFSIZE ((size_t)10)
#define MAXPENDING 10
int get_server_sock(char* service){
	struct addrinfo addr_config;
	memset(&addr_config, 0, sizeof(addr_config));
	addr_config.ai_family = AF_INET6;
	addr_config.ai_flags = AI_PASSIVE;
	addr_config.ai_socktype = SOCK_STREAM;
	addr_config.ai_protocol = IPPROTO_TCP;
	struct addrinfo* addr_list;
	int rtn_value = getaddrinfo(NULL, service, &addr_config, &addr_list);
	if(rtn_value!=0){
		return -1;
	}
	int fd = -1;
	for(struct addrinfo* l_item=addr_list;l_item!=NULL;l_item=l_item->ai_next){
		fd = socket(l_item->ai_family,l_item->ai_socktype,l_item->ai_protocol);
		if(fd<0){
			continue;
		}
		int bind_result = bind(fd, l_item->ai_addr, l_item->ai_addrlen);
		int listen_result = listen(fd, MAXPENDING);
		if((bind_result == 0) && (listen_result == 0)){
				int val = 1;
				setsockopt(fd,
						   SOL_SOCKET,
						   SO_REUSEADDR,
						   &val, // Enable the option
						   sizeof(val));
				break;
		}
	}
	return fd;
}


int main() {
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;
	server_fd = get_server_sock("6379");
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}
	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);
	int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
	char* msg = "+PONG\r\n";
	printf("Client connected\n");
	char* buff = calloc(BUFSIZE,sizeof(char));
	while(recv(client_fd,buff,(size_t)BUFSIZE-1,0)!=EOF){
		send(client_fd,msg,strlen(msg),0);
	}
	close(server_fd);
	close(client_fd);
	free(buff);

	return 0;
}
