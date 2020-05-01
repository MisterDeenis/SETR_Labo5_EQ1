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
#include <sys/time.h>
#include <vorbis/vorbisenc.h>
#include "audio.h"

const char debugPipeSrc[] = "/tmp/audioEmitterPipe\0";

int progOK = 1;

void gereSignal(int signo) {
    if (signo == SIGTERM || signo == SIGPIPE){
        progOK = 0;
    }
}

int main(int argc, char* argv[]){

    char *sval = NULL;
    int schedPolicy = SCHED_RR;
    int isDebug = 0;

    int argPos = 0;
    int option;

    if(strcmp(argv[1], "--debug\0") == 0){
        printf("debug option\n");
        isDebug = 1;
        schedPolicy = SCHED_RR;
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

    const char *readerPipe = NULL;
    if(isDebug == 1){
        readerPipe = debugPipeSrc;
    }else{
        readerPipe = argv[argc - 1];
    }

    //init signal
    signal(SIGTERM, gereSignal);
    signal(SIGPIPE, gereSignal);

    //pipe init
    int pipeFd = init_reader_pipe(readerPipe);
    if(pipeFd < 0){
        printf("Erreur init reader pipe\n");
        exit(1);
    }

    //sched init
    struct sched_param param;
    param.sched_priority = 99;
    if(sched_setscheduler(0, schedPolicy, &param) != 0){
            printf("Erreur lors du changement d'ordonnancement. %i\n",errno);
            exit(1);
    }

    char *sampleBuf = (char*) malloc(sizeof(char) * NBR_SAMPLE);
    //init speaker
    unsigned int rate = TRSF_RATE;
    snd_pcm_uframes_t frames = NBR_SAMPLE;
    snd_pcm_t *playback_handle = audio_init(DEV_SPEAKER, 1, 1, &frames, &rate);

    int nbSample = 0;

    struct timeval timeLast;
    gettimeofday(&timeLast, NULL);
    while(progOK){
        int bytes_read;
        if ((bytes_read = pipe_read(pipeFd, sampleBuf, NBR_SAMPLE*2)) < 0){
            fprintf(stderr, "audioEmitter : error reading pipe\n");
            exit(1);
        }
        //Code pour faire jouer le son

        if (bytes_read > 0){
            nbSample += bytes_read;
            audio_write(playback_handle, sampleBuf, NBR_SAMPLE);
        }

        struct timeval timeCourant;
        gettimeofday(&timeCourant, NULL);
        int intervalTimeStat = timeCourant.tv_sec - timeLast.tv_sec;
        if(intervalTimeStat > 1){
            fprintf(stderr, "bytes/s : %i\n", nbSample/intervalTimeStat/2);
            timeLast = timeCourant;
            nbSample = 0;
        }

        sched_yield(); //peut-être remplacer par un sleep...
    }
    
    close_reader_pipe(pipeFd);
    audio_destroy(playback_handle);
    free(sampleBuf);
}
