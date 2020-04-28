#ifndef SOCKET_COM
#define SOCKET_COM

#include <stdio.h>

int init_client_socket();

int init_server_socket();

int socket_read(int fd, char *buffer, size_t len);

int socket_write(int fd, char *buffer, size_t len);

void close_client_socket(int fd);

void close_server_socket(int fd);

#endif