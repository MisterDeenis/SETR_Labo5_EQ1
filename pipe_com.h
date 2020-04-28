#ifndef PIPE_COM
#define PIPE_COM

#include <stdio.h>

int init_writer_pipe(const char *pathname);

int init_reader_pipe(const char *pathname);

int pipe_read(int fd, char *buffer, size_t len);

int pipe_write(int fd, char *buffer, size_t len);

void close_writer_pipe(const char *pathname, int fd);

void close_reader_pipe(int fd);

#endif