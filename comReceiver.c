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

const char debugPipeDest[] = "/tmp/audioEmitterPipe\0";

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
        printf("debug option\n");
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
                    printf("Argument inconnu : %d\n", option);
                    exit(1);
            }
        }
    }

    const char *writerPipe = NULL;
    if(isDebug == 1){
        writerPipe = debugPipeDest;
    }else{
        writerPipe = argv[argc - 1];
    }

    //init signal
    signal(SIGTERM, gereSignal);
    signal(SIGPIPE, gereSignal);

    //pipe init
    int pipeFd = init_writer_pipe(writerPipe);
    if(pipeFd < 0){
        printf("Erreur init reader pipe\n");
        exit(1);
    }

    //socket init
    int socketFd = init_server_socket();
    if(socketFd < 0){
        printf("Erreur init writer socket\n");
        exit(1);
    }

    //sched init
    if(sched_setscheduler(0, schedPolicy, &param) != 0){
            printf("Erreur lors du changement d'ordonnancement.\n");
            exit(1);
    }

    char *sampleBuf = (char*) malloc(sizeof(char) * NBR_SAMPLE);


    while(progOK){
        int read = socket_read(socketFd, sampleBuf, NBR_SAMPLE*2);
        if(read > 0){
            //fprintf(stderr, "sampleBuf = %s\n", sampleBuf);
            pipe_write(pipeFd, sampleBuf, NBR_SAMPLE*2);
        }      
        sched_yield(); //peut-être remplacer par un sleep...
    }

    close_writer_pipe(writerPipe, pipeFd);
    close_server_socket(socketFd);
    free(sampleBuf);
}