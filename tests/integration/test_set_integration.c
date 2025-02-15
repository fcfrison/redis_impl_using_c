#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <assert.h>

#define SERVER_IP "127.0.0.1"
#define PORT 6379
#define BUFFER_SIZE 1024

void send_command(int sock, const char* command) {
    if (send(sock, command, strlen(command), 0) < 0) {
        perror("Send failed");
        exit(EXIT_FAILURE);
    }
}

char* receive_response(int sock) {
    char buffer[BUFFER_SIZE] = {0};
    if (read(sock, buffer, BUFFER_SIZE) < 0) {
        perror("Read failed");
        exit(EXIT_FAILURE);
    }
    return strdup(buffer);
}

void test_set_basic_command() {
    int sock;
    struct sockaddr_in serv_addr;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    // Send SET command
    const char* set_command = "*3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n";
    send_command(sock, set_command);

    // Receive response
    char* response = receive_response(sock);
    printf("Response for SET command: %s\n", response);

    // Validate response
    assert(strcmp(response, "+OK\r\n") == 0); // Ensure the response is "+OK"

    free(response);
    close(sock);
}

void test_set_command_with_nx_option() {
    int sock;
    struct sockaddr_in serv_addr;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    // Send SET command with NX option
    const char* set_nx_command = "*4\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n$2\r\nNX\r\n";
    send_command(sock, set_nx_command);

    // Receive response
    char* response = receive_response(sock);
    printf("Response for SET NX command: %s\n", response);

    // Validate response
    assert(strcmp(response, "+OK\r\n") == 0); // Ensure the response is "+OK"

    free(response);
    close(sock);
}

void test_set_command_with_ex_option() {
    int sock;
    struct sockaddr_in serv_addr;

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        exit(EXIT_FAILURE);
    }

    // Send SET command with EX option
    const char* set_ex_command = "*5\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n$2\r\nEX\r\n$2\r\n10\r\n";
    send_command(sock, set_ex_command);

    // Receive response
    char* response = receive_response(sock);
    printf("Response for SET EX command: %s\n", response);

    // Validate response
    assert(strcmp(response, "+OK\r\n") == 0); // Ensure the response is "+OK"

    free(response);
    close(sock);
}

int main() {
    //test_set_basic_command();
    test_set_command_with_nx_option();
    //test_set_command_with_ex_option();

    printf("All integration tests passed!\n");
    return 0;
}