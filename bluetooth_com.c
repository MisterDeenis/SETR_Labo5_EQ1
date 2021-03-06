
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
#include <bluetooth/hci_lib.h>

#include "constants.h"
#include "errno.h"

//add client : B8:27:EB:BE:49:8A
//add serveur : B8:27:EB:0C:71:D7 

int init_server_bluetooth(){
    
    int ret_fd;
    int sock_fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if(sock_fd == -1){
        printf("Erreur ouverture socket\n");
        exit(1);
    }

    bdaddr_t bdaddr_any = {0,0,0,0,0,0};
    struct sockaddr_rc addr;
    addr.rc_family = AF_BLUETOOTH;
    addr.rc_bdaddr = bdaddr_any;
    addr.rc_channel = htobs(1);
    int alen = sizeof(addr);

    if(bind(sock_fd, (struct sockaddr *)&addr, alen) < 0)
    {
      perror("bind");
      exit(1);
    }

    int ret = listen(sock_fd, 4);
    fprintf(stderr, "Retour listen %i\n", ret);

    fprintf(stderr, "Waiting for client\n");
    struct sockaddr_rc raddr;
    if((ret_fd = accept(sock_fd, (struct sockaddr *) &raddr, (socklen_t *)&alen)) < 0){
        printf("Erreur lors du accept\n");
        exit(1);
    }

    fprintf(stderr, "Client connected\n");

    return ret_fd;
}

int init_client_bluetooth(){

    struct sockaddr_rc laddr, raddr;

    int sock_fd = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
    if(sock_fd < 0){
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
    str2ba(ADDR_CLIENT,&laddr.rc_bdaddr);
    laddr.rc_channel = 1;
    
    fprintf(stderr, "Trying to connect to server\n");
    
    int ret = 0;
    while((ret = connect(sock_fd, (struct sockaddr *)&laddr, sizeof(laddr))) < 0){
        usleep(50);
        fprintf(stderr, "Retour de connect %i\n", ret);
        fprintf(stderr, "errno : %i\n", errno);
    }
    fprintf(stderr, "Connected to server\n");
    
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