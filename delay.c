// Gestion des ressources et permissions
#include <sys/resource.h>

// Nécessaire pour pouvoir utiliser sched_setattr et le mode DEADLINE
#include <sched.h>

#include <signal.h>

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "pipe_com.h"
#include "constants.h"

char debugPipeSrc[] = "/tmp/audioReceiverPipe\0";
char debugPipeDest[] = "/tmp/comEmitterPipe\0";

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
	int delay_time = 0;
	int delay_factor = 0;

    int argPos = 0;
    int option;

    if(strcmp(argv[1], "--debug\0") == 0){
        printf("debug option\n");
        isDebug = 1;
        schedPolicy = SCHED_OTHER;
    }else{
        while((option = getopt(argc, argv, "s:t:f:")) != -1){
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
				case 't':
					delay_time = atoi(optarg);
					break;
				case 'f':
					delay_factor = atoi(optarg);
					break;
                default:
                    printf("Argument inconnu : %d\n", option);
                    exit(1);
            }
        }
    }

    char *readerPipe = NULL;
    char *writerPipe = NULL;
    if(isDebug == 1){
        readerPipe = debugPipeSrc;
        writerPipe = debugPipeDest;
    }else{
        readerPipe = debugPipeSrc;
        writerPipe = debugPipeDest;
    }

    //init signal
    signal(SIGTERM, gereSignal);
    signal(SIGPIPE, gereSignal);

    //reader pipe init
    int readerFd = init_reader_pipe(readerPipe);
    if(readerFd < 0){
        printf("Erreur init reader pipe\n");
        exit(1);
    }

    //writer pipe init
    int writerFd = init_writer_pipe(writerPipe);
    if(writerFd < 0){
        printf("Erreur init writer pipe\n");
        exit(1);
    }

    //sched init
    struct sched_param param;
    param.sched_priority = 99;
    if(sched_setscheduler(0, schedPolicy, &param) != 0){
            printf("Erreur lors du changement d'ordonnancement.\n");
            exit(1);
    }

	int buf_head = 0;
	uint16_t *bufMem = (uint16_t*) malloc(sizeof(uint16_t) * delay_time);
	memset(bufMem, 0, sizeof(char) * delay_time);
    char *sampleBuf = (char*) malloc(sizeof(char) * NBR_SAMPLE);


    while(progOK){
        pipe_read(readerFd, sampleBuf, NBR_SAMPLE);
        
		uint16_t *sampleBuf_16 = (uint16_t*) sampleBuf;
		int nbrSample = (NBR_SAMPLE / 2);
		// Traitement pour mettre le délais
        for(int i = 0; i < nbrSample; i++){
            uint16_t sample = sampleBuf_16[i];
            sample = sample + ((bufMem[buf_head] * (uint16_t)delay_factor) / (uint16_t)100);
            bufMem[buf_head] = sample;
            buf_head = (buf_head + 1) % delay_time;
        }
		
        pipe_write(writerFd, sampleBuf, NBR_SAMPLE);
        sched_yield(); //peut-être remplacer par un sleep...
    }

    close_reader_pipe(readerFd);
    close_writer_pipe(writerPipe, writerFd);
    free(sampleBuf);
}