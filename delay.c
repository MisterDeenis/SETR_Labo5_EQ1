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
					delay_time = atoi(&optarg);
					break;
				case 'f':
					delay_factor = atoi(&optarg);
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

	int buf_head = 0;
	char *bufMem = (char*) malloc(sizeof(char) * delay_time);
	memset(bufMem, 0, sizeof(char) * delay_time);
    char *sampleBuf = (char*) malloc(sizeof(char) * NBR_SAMPLE);


    while(progOK){
        pipe_read(readerFd, sampleBuf, NBR_SAMPLE);
        
		// Traitement pour mettre le délais
		for(int i = 0; i < NBR_SAMPLE; i++){
			int pos = i - delay_time;
			int delayVal = 0;
			if(pos < 0){
				pos = (buf_head - pos) % delay_time;
				delayVal = bufMem[pos];
			}else{
				delayVal = sampleBuf[pos];
			}
			
			sampleBuf[i] = sampleBuf[i] + (delayVal * delay_factor / 100);
		}
		
		//Traitement pour mettre en mémoire les échantillons
		int nbrSampleToCpy = (delay_time < NBR_SAMPLE) ? delay_time : NBR_SAMPLE;
		//première copie
		if(buf_head + nbrSampleToCpy > delay_time){
			//2 copies doivent être faites
			int nbrSampleToCpyFirst = delay_time - buf_head;
			memcpy((bufMem + buf_head), sampleBuf, nbrSampleToCpyFirst);
			nbrSampleToCpy -= nbrSampleToCpyFirst;
			memcpy(bufMem, (sampleBuf + nbrSampleToCpyFirst), nbrSampleToCpy);
			buf_head = (buf_head + nbrSampleToCpy) % delay_time;			
		}else{
			//1 seule copie est faites
			memcpy((bufMem + buf_head), sampleBuf, nbrSampleToCpy);
			buf_head = (buf_head + nbrSampleToCpy) % delay_time
		}
		
		
        

        pipe_writer(writerFd, sampleBuf, NBR_SAMPLE);
        sched_yield(); //peut-être remplacer par un sleep...
    }

    close_reader_pipe(readerPipe);
    close_writer_pipe(writePipe, writerFd);
    free(sampleBuf);
}