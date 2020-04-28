
#include "pipe_com.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "errno.h"

int init_writer_pipe(const char *pathname){
    //Il va y avoir une erreur de pipe déjà créé faire un unlink

    int sucess = mkfifo(pathname, 0666);
    if(sucess < 0){
        unlink(pathname);
        mkfifo(pathname, 0666);
    }
    return open(pathname, O_WRONLY);
}

int init_reader_pipe(const char *pathname){

    int fd = -1;
    do {
        fd = open(pathname, O_RDONLY);
        usleep(50);
    }while(fd == -1);

    return fd;
}

int pipe_read(int fd, char *buffer, size_t len){
    return read(fd, buffer, len);
}

int pipe_write(int fd, char *buffer, size_t len){
    return write(fd, buffer, len);
}

void close_writer_pipe(const char *pathname, int fd){
    close(fd);
    unlink(pathname);
}

void close_reader_pipe(int fd){
    close(fd);
}