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

const char debugPipeSrc[] = "/audioReceiverPipe\0";
const char debugPipeDest[] = "/comEmitterPipe\0";

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
	float filter_factor = 0;

    int argPos = 0;
    int option;

    if(strcmp(argv[1], "--debug\0") == 0){
        printf("debug option\n");
        isDebug = 1;
        schedPolicy = SCHED_OTHER;
    }else{
        while((option = getopt(argc, argv, "s:f:")) != -1){
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
				case 'f':
					filter_factor = (float) atoi(&optarg);
					break;
                default:
                    printf("Argument inconnu : %d\n", option);
                    exit(1);
            }
        }
    }

    const char *readerPipe = NULL;
    const char *writePipe = NULL;
    if(isDebug == 1){
        readerPipe = debugPipeSrc;
        writerPipe = debugPipeDest;
    }else{
        readerPipe = argv[argc - 2];
        writerPipe = argv[argc - 1];
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

	float lastInput = 0;
	float lastOutput = 0;
	memset(bufMem, 0, sizeof(char) * delay_time);
    char *sampleBuf = (char*) malloc(sizeof(char) * NBR_SAMPLE);
	char *sampleBuf_out = (char*) malloc(sizeof(char) * NBR_SAMPLE);

    while(progOK){
        pipe_read(readerFd, sampleBuf, NBR_SAMPLE);
        
		uint16_t *sampleBuf_16 = (uint16_t*) sampleBuf;
		uint16_t *sampleBuf_out16 = (uint16_t*) sampleBuf_out;
		int nbrSample = (NBR_SAMPLE / 2);
		// Traitement pour mettre le délais
		for(int i = nbrSample - 1; i >= 0; i--){
			float sample = (float) sampleBuf_16[i];
			if(i = nbrSample - 1){
				sample = ((lastInput - sample) * 1 - (filter_factor / 100)) + (lastOutput * (filter_factor / 100));
			}else{
				sample = ((sampleBuf[i + 1] - sample) * 1 - (filter_factor / 100)) + (sampleBuf_out[i + 1] * (filter_factor / 100));
			}			
			sampleBuf_out[i] = (uint16_t) sample;			
		}
		
		lastInput = (float) sampleBuf_16[0];
		lastOutput = (float) sampleBuf_out16[0];
		
        pipe_writer(writerFd, sampleBuf_out, NBR_SAMPLE);
        sched_yield(); //peut-être remplacer par un sleep...
    }

    close_reader_pipe(readerPipe);
    close_writer_pipe(writePipe, writerFd);
    free(sampleBuf);
}