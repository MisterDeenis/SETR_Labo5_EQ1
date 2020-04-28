
#include "socket_com.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "constants.h"
#include "errno.h"

int sock_fd;

int init_server_socket(){
    int ret_fd;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd == -1){
        fprintf(stderr, "Erreur ouverture socket\n");
        exit(1);
    }

    struct sockaddr_in sockInfo;
    memset(&sockInfo, 0, sizeof(sockInfo));
    sockInfo.sin_family = AF_INET;
    sockInfo.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    sockInfo.sin_port = htons(PORT);
    
    if(bind(sock_fd, (struct sockaddr *) &sockInfo, sizeof(sockInfo)) < 0){
        fprintf(stderr, "Erreur de bind du socket \n");
        exit(1);
    }

    if(listen(sock_fd, 3) < 0){
        fprintf(stderr, "Erreur lors du listen\n");
        exit(1);
    }

    fprintf(stderr, "Waiting for client to connect\n");
    int addrLen = sizeof( (struct sockaddr *) &sockInfo);;
    if((ret_fd = accept(sock_fd, (struct sockaddr *) &sockInfo, (socklen_t *)&addrLen)) < 0){
        fprintf(stderr, "Erreur lors du accept : %i\n",errno);
        exit(1);
    }
    fprintf(stderr, "Connexion with client obtained\n");

    return ret_fd;
}

int init_client_socket(){
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd == -1){
        fprintf(stderr, "Erreur ouverture socket\n");
        exit(1);
    }

    struct sockaddr_in sockInfo;
    memset(&sockInfo, 0, sizeof(sockInfo));
    sockInfo.sin_family = AF_INET;
    sockInfo.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    sockInfo.sin_port = htons(PORT);

    fprintf(stderr, "Waiting for server\n");
    while(connect(sock_fd, (struct sockaddr *)&sockInfo, sizeof(sockInfo)) < 0){
        usleep(50);
    }
    fprintf(stderr, "Server connexion acquired\n");

    return sock_fd;
}

int socket_read(int fd, char *buffer, size_t len){
    return read(fd, buffer, len);
}

int socket_write(int fd, char *buffer, size_t len){
    return write(fd, buffer, len);
}

void close_client_socket(int fd){
    close(fd);
}

void close_server_socket(int fd){
    close(fd);
}