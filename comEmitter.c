// Gestion des ressources et permissions
#include <sys/resource.h>

// Nécessaire pour pouvoir utiliser sched_setattr et le mode DEADLINE
#include <sched.h>

#include <signal.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pipe_com.h"
#include "socket_com.h"
#include "constants.h"

const char debugPipeSrc[] = "/tmp/audioReceiverPipe\0";

int progOK = 1;

void gereSignal(int signo) {
    if (signo == SIGTERM || signo == SIGPIPE){
        progOK = 0;
    }
}

int main(int argc, char* argv[]){

    char *sval = NULL;
    int schedPolicy = SCHED_OTHER;
    int isDebug = 0;
    struct sched_param param;

    int argPos = 0;
    int option;

    if(strcmp(argv[1], "--debug\0") == 0){
        printf("comEmitter : debug option\n");
        isDebug = 1;
        schedPolicy = SCHED_RR;
        param.sched_priority = 99;
    }else{
        while((option = getopt(argc, argv, "s:")) != -1){
            argPos++;
            switch (option){
                case 's': 
                    sval = optarg;
                    if(strcmp(sval, "NORT\0") == 0){
                        printf("nort\n");
                        schedPolicy = SCHED_OTHER;
                    }else if(strcmp(sval, "RR\0") == 0){
                        printf("rr\n");
                        schedPolicy = SCHED_RR;
                    }else if(strcmp(sval, "FIFO\0") == 0){
                        printf("fifo\n");
                        schedPolicy = SCHED_FIFO;
                    }
                    break;
                default:
                    printf("comEmitter : Argument inconnu : %d\n", option);
                    exit(1);
            }
        }
    }

    const char *readerPipe = NULL;
    if(isDebug == 1){
        readerPipe = debugPipeSrc;
    }else{
        readerPipe = argv[argc - 1];
    }

    //pipe init
    int pipeFd = init_reader_pipe(readerPipe);
    if(pipeFd < 0){
        printf("comEmitter : Erreur init reader pipe\n");
        exit(1);
    }

    //socket init
    int socketFd = init_client_socket();
    if(socketFd < 0){
        printf("comEmitter : Erreur init writer socket\n");
        exit(1);
    }

    //sched init
    if(sched_setscheduler(0, schedPolicy, &param) != 0){
            printf("comEmitter : Erreur lors du changement d'ordonnancement.\n");
            exit(1);
    }

    char *sampleBuf = (char*) malloc(sizeof(char) * NBR_SAMPLE);


    while(progOK){
        int read = pipe_read(pipeFd, sampleBuf, NBR_SAMPLE*2);
        if(read > 0){
            socket_write(socketFd, sampleBuf, NBR_SAMPLE*2);
        }      
        sched_yield(); //peut-être remplacer par un sleep...
    }

    close_writer_pipe(readerPipe, pipeFd);
    close_client_socket(socketFd);
    free(sampleBuf);
}