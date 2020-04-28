// Gestion des ressources et permissions
#include <sys/resource.h>

// Nécessaire pour pouvoir utiliser sched_setattr et le mode DEADLINE
#include <sched.h>

#include <signal.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pipe_com.h"
#include "constants.h"

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <vorbis/vorbisenc.h>
#include "audio.h"

#include "errno.h"


const char debugPipeDest[] = "/tmp/audioReceiverPipe\0";

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
        printf("audioReceiver : debug option\n");
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
                    printf("audioReceiver : Argument inconnu : %d\n", option);
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
        printf("audioReceiver : Erreur init writer pipe\n");
        exit(1);
    }

    //sched init
    int test;
    if((test = sched_setscheduler(0, schedPolicy, &param)) != 0){
            printf("audioReceiver : Erreur lors du changement d'ordonnancement : %i.\n", test);
            exit(1);
    }

    char *sampleBuf = (char*) malloc(sizeof(char) * NBR_SAMPLE);

    //init le micro
    unsigned int rate = TRSF_RATE;
    snd_pcm_uframes_t frames = NBR_SAMPLE;
    snd_pcm_t *capture_handle = audio_init(DEV_MICRO, 0, 1, &frames, &rate);
    int buffer_frames = NBR_SAMPLE;


    while(progOK){
        //Code pour aller chercher la trame d'échantillions
        //mettre les sample dans sampleBuf

        audio_read(capture_handle, sampleBuf, buffer_frames);
        //fprintf(stderr, "sampleBuf = %s\n", sampleBuf);

        pipe_write(pipeFd, sampleBuf, buffer_frames*2);

        sched_yield(); //peut-être remplacer par un sleep...
    }
    audio_destroy(capture_handle);

    close_writer_pipe(writerPipe, pipeFd);
    free(sampleBuf);

    exit(0);
}