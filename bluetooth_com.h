#ifndef BLUETOOTH_COM
#define BLUETOOTH_COM


#include <stdio.h>

#define BLUETOOTH_IP_ADDR "0.0.0.0"

int init_server_bluetooth();

int init_client_bluetooth();

int bluetooth_write(int fd, char *buffer, size_t len);

int bluetooth_read(int fd, char *buffer, size_t len);

void bluetooth_writer_close(int fd);

void bluetooth_reader_close(int fd);

#endif