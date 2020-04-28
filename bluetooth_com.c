
#include "socket_com.h"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <bluetooth/hci.h>

int sock_fd;

int init_server_bluetooth(){
    
    int ret_fd;
    sock_fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if(sock_fd == -1){
        printf("Erreur ouverture socket\n");
        exit(1);
    }

    struct sockaddr_rc addr;
    addr.rc_family = AF_BLUETOOTH;
    bacpy(&addr.rc_bdaddr, BDADDR_ANY);
    addr.rc_channel = htobs(4);
    int alen = sizeof(addr);

    if(bind(sock, (struct sockaddr *)&addr, alen) < 0)
    {
      perror("bind");
      exit(1);
    }

    listen(sock, 3);

    if((ret_fd = accept(sock_fd, (struct sockaddr *) &addr, (socklen_t *)&alen)) < 0){
        printf("Erreur lors du accept\n");
        exit(1);
    }

    return ret_fd;
}

int init_client_bluetooth(){

    struct sockaddr_rc laddr, raddr;


    sock_fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if(sock_fd == -1){
        printf("Erreur ouverture socket\n");
        exit(1);
    }

    struct hci_dev_info di;
    if(hci_devinfo(0, &di) < 0) 
    {
      printf("Erreur hci\n");
      exit(1);
    }

    laddr.rc_family  = AF_BLUETOOTH;
    laddr.rc_bdaddr = di.bdaddr;
    laddr.rc_channel = 0;

    raddr.rc_family = AF_BLUETOOTH;
    str2ba(argv[1],&raddr.rc_bdaddr);
    raddr.rc_channel = 4;
    
    if(bind(sock_fd, (struct sockaddr *)&laddr, sizeof(laddr)) < 0)
    {
      printf("Erreur, bind socket client");
      exit(1);
    }

    while(connect(sock_fd, (struct sockaddr *)&raddr, sizeof(raddr)) < 0){
        usleep(50);
        fprintf(stderr, "Erreur lors du connect du socket\n");
    }
    
    return sock_fd;
}

int bluetooth_write(int fd, char *buffer, size_t len){
    return write(fd, buffer, len);
}

int bluetooth_read(int fd, char *buffer, size_t len){
    return read(fd, buffer, len);
}

void bluetooth_writer_close(int fd){
    close(fd);
}

void bluetooth_reader_close(int fd){
    close(fd);
}