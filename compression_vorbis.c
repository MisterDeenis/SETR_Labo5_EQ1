#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <sys/time.h>
#include "compression.h"
#include "constants.h"

ogg_stream_state os; /* take physical pages, weld into a logical
                        stream of packets */
ogg_page         og; /* one Ogg bitstream page.  Vorbis packets are inside */
ogg_packet       op; /* one raw packet of data for decode */

vorbis_info      vi; /* struct that stores all the static vorbis bitstream
                      settings */
vorbis_comment   vc; /* struct that stores all the user comments */

vorbis_dsp_state vd; /* central working state for the packet->PCM decoder */
vorbis_block     vb; /* local working space for packet->PCM decode */
int eos=0;

void init_compression(int outFd){
  int err;
  
  vorbis_info_init(&vi);

  if ((err = vorbis_encode_init(&vi,1,44100, 128000, -1, -1))) {
      printf("vorbis_encode_init failed\n");
      exit(1);
  }

  vorbis_comment_init(&vc);

    /* set up the analysis state and auxiliary encoding storage */
  vorbis_analysis_init(&vd,&vi);
  vorbis_block_init(&vd,&vb);

  srand(time(NULL));
  ogg_stream_init(&os,rand());

  {
    ogg_packet header;
    ogg_packet header_comm;
    ogg_packet header_code;

    vorbis_analysis_headerout(&vd,&vc,&header,&header_comm,&header_code);
    ogg_stream_packetin(&os,&header); /* automatically placed in its own
                                          page */
    ogg_stream_packetin(&os,&header_comm);
    ogg_stream_packetin(&os,&header_code);

    /* This ensures the actual
      * audio data will start on a new page, as per spec
      */
    while(!eos){
        int result=ogg_stream_flush(&os,&og);
        if(result==0)break;
        write(outFd, og.header, og.header_len);
        write(outFd, og.body, og.body_len);
    }

  }
}

void compress_audio(char* inBuffer, size_t inBuffer_size, int outFd){
  int buffer_frames = inBuffer_size/2;

  float **vorbis_buffer=vorbis_analysis_buffer(&vd,inBuffer_size/2);
  for(int i = 0; i < (buffer_frames); i++){
      int16_t *sample = (int16_t*) &inBuffer[i*2];
      vorbis_buffer[0][i] = *sample / 32768.f;
  }

  long long start_time = (unsigned)time(NULL);
  vorbis_analysis_wrote(&vd,buffer_frames);
  while(vorbis_analysis_blockout(&vd,&vb)==1){

      /* analysis, assume we want to use bitrate management */
      vorbis_analysis(&vb,NULL);
      vorbis_bitrate_addblock(&vb);

      while(vorbis_bitrate_flushpacket(&vd,&op)){

          /* weld the packet into the bitstream */
          ogg_stream_packetin(&os,&op);

          /* write out pages (if any) */
          while(!eos){
            int result=ogg_stream_pageout(&os,&og);
            if(result==0)break;
            write(outFd, og.header, og.header_len);
            write(outFd, og.body, og.body_len);

            /* this could be set above, but for illustrative purposes, I do
                it here (to show that vorbis does know where the stream ends) */

            if(ogg_page_eos(&og))eos=1;
          }
      }
  }
  printf("time(ms): %lu\n", (unsigned)time(NULL)-start_time);
}

void close_compression(){
  vorbis_analysis_wrote(&vd,0);
  ogg_stream_clear(&os);
  vorbis_block_clear(&vb);
  vorbis_dsp_clear(&vd);
  vorbis_comment_clear(&vc);
  vorbis_info_clear(&vi);
  eos=0;
}