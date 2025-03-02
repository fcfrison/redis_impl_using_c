#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"  // localhost
#define PORT 6379
#define BUFFER_SIZE 1024

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    
    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error\n");
        return -1;
    }
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    
    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported\n");
        return -1;
    }
    
    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("Connection Failed\n");
        return -1;
    }
    //strcpy(buffer,"*4\r\n$1\r\n1\r\n$1\r\n2\r\n*2\r\n$1\r\na\r\n$1\r\nb\r\n$1\r\n3\r\n");
    //strcpy(buffer,"*4\r\n$1\r\n1\r\n$1\r\n2\n*2\r\n$1\r\na\r\n$1\r\nb\r\n$1\r\n3\r\n");
    //strcpy(buffer,"*2\r\n$4\r\nLLEN\r\n$6\r\nmylist\r\n");
    //strcpy(buffer,"*1\r\n$4\r\nPING\r\n");
    //strcpy(buffer,"*2\r\n$4\r\nECHO\r\n$3\r\nhey\r\n");
    /*
    char* cmd_str[] = {
        "*2\r\n$4\r\nECHO\r\n$3\r\nhey\r\n",
        "*2\r\n$4\r\nECHO\r\n$5\r\nhello\r\n",
        "*2\r\n$4\r\nECHO\r\n$10\r\nredis-test\r\n",
        "*2\r\n$4\r\nECHO\r\n$9\r\n123456789\r\n",
        "*2\r\n$4\r\nECHO\r\n$0\r\n\r\n",
        "*2\r\n$4\r\nECHO\r\n$13\r\nlonger-string\r\n",
        "*2\r\n$4\r\nECHO\r\n$1\r\na\r\n",
        "*2\r\n$4\r\nECHO\r\n$2\r\nok\r\n",
        "*2\r\n$4\r\nECHO\r\n$6\r\ngoodbye\r\n",
        "*2\r\n$4\r\nECHO\r\n$4\r\ntest\r\n"

    };
    char* cmd_str_a[] = {
        "*2\r\n$4\r\nECHO\r\n$12\r\nLine1\nLine2\r\n",  // Comando ECHO com múltiplas linhas
        "*2\r\n$4\r\nECHO\r\n$15\r\nHey, how are you?\r\n",  // Comando ECHO com uma pergunta
        "*2\r\n$4\r\nECHO\r\n$17\r\nECHOing the echo!\r\n",  // Comando ECHO com uma frase longa
        "*2\r\n$4\r\nECHO\r\n$1\r\na\r\n"  // Comando ECHO com uma única letra
    };
    char* cmd_str_a[] = {
        "*2\r\n$4\r\nECHO\r\n$3\r\nhey\r\n*2\r\n$4\r\nECHO\r\n$5\r\nhello\r\n*2\r\n$4\r\nECHO\r\n$10\r\nredis-test\r\n*2\r\n$4\r\nECHO\r\n$9\r\n123456789\r\n*2\r\n$4\r\nECHO\r\n$0\r\n\r\n*2\r\n$4\r\nECHO\r\n$13\r\nlonger-string\r\n*2\r\n$4\r\nECHO\r\n$1\r\na\r\n*2\r\n$4\r\nECHO\r\n$2\r\nok\r\n*2\r\n$4\r\nECHO\r\n$6\r\ngoodbye\r\n*2\r\n$4\r\nECHO\r\n$4\r\ntest\r\n"

    };
    */
    char* cmd_str_a[] = {
        "*5\r\n$3\r\nSET\r\n$10\r\nanotherkey\r\n$23\r\nwill expire in a minute\r\n$2\r\nEX\r\n$2\r\n60\r\n",
    };
    unsigned char size = 1;
    ssize_t rtn     = 0;
    for(unsigned char i=0;i<size;i++){
        size_t  b_to_send = strlen(cmd_str_a[i]);
        ssize_t b_sent  = 0;
        ssize_t rmn_bt  = b_to_send;
        char*   start_pos = cmd_str_a[i];
        printf("The size of the string is: %ld\n",b_to_send);
        while(b_sent<b_to_send){
            rtn = send(sock, start_pos, rmn_bt, 0);
            //printf("bytes send: %d\n",rtn);
            if(!rtn || rtn==-1){
                perror("Error: ");
                close(sock);
                puts("Client is over");
                return 0;
            }
            b_sent+=rtn;
            rmn_bt = b_to_send - b_sent;
            start_pos+=rtn;
            perror("Error: ");
        }
    }
    char* buf  = calloc(1024, sizeof(char));
    while(1){
        puts("Client waiting");
        rtn = recv(sock,buf,1024,0);
        printf("Bytes received: %d\n",rtn);
        if(!rtn || rtn==-1){
            perror("Error: ");
            break;
        }
        puts(buf);
    }
    close(sock);
    return 0;
}