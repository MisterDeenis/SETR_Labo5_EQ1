#include <stdio.h>
#include <stdlib.h>
#include "compression.h"
#include "constants.h"

OpusDecoder* decoder_init(int n_channels, int sample_rate)
{
    OpusDecoder* decoder = NULL;
    int size = 0;
    int ret;
    size = opus_decoder_get_size(n_channels); /* max 2 channels */
    decoder = (OpusDecoder*)malloc(size);
    if (!decoder) {
      printf("fail to create encoder");
      exit(1);
    }
    ret = opus_decoder_init(decoder, sample_rate, n_channels);
    if (ret != 0) {
      printf("fail to init %d", ret);
      exit(ret);
    }
    return decoder;
}

int decode_audio(OpusDecoder* decoder, const unsigned char* inBuffer, int taille_inBuffer, opus_int16* outBuffer, int taille_outBuffer){
  //TODO use NULL when packet lost for inBuffer

  int ret = opus_decode(decoder, inBuffer, taille_inBuffer, outBuffer, taille_outBuffer, 0);
  if (ret <= 0) {
      printf("fail to decode : %d\n", ret);
      return -1;
  }  

  return 0;
}

void close_decoder(OpusDecoder* decoder){
  free(decoder);
}



OpusEncoder* encoder_init(int n_channels, int sample_rate){
  OpusEncoder *encoder = NULL;
  int size = 0;
  size = opus_encoder_get_size(n_channels);
  encoder = (OpusEncoder*)malloc(size);
  if (!encoder) {
    printf("fail to create encoder");
    exit(1);
  }
  // Ici on détermine le sample rate, le nombre de channel et un option pour la méthode d'encodage.
  // OPUS_APPLICATION_RESTRICTED_LOWDELAY nous donne la plus faible latence, mais vous pouvez essayer d'autres options.
  int ret = opus_encoder_init(encoder, sample_rate, n_channels, OPUS_APPLICATION_RESTRICTED_LOWDELAY);
  if (ret != 0) {
    printf("fail to init %d", ret);
    exit(1);
  }

  /* paramètres de l'encodeur */
  // Bitrate du signal encodé : Vous devez trouver le meilleur compromis qualité/transmission.
  ret = opus_encoder_ctl(encoder, OPUS_SET_BITRATE(128000));
  if (ret != 0) {
      printf("fail to set bitrate");
      exit(1);
  }

  // Complexité de l'encodage, permet un compromis qualité/latence
  ret = opus_encoder_ctl(encoder, OPUS_SET_COMPLEXITY(0));
  if (ret != 0) {
    printf("fail to set complexity");
    exit(1);
  }

  // Variation du bitrate.
  ret = opus_encoder_ctl(encoder, OPUS_SET_VBR(0));
  if (ret != 0) {
    printf("fail to set vbr");
    exit(1);
  }
  // type de signal à encoder (dans notre cas il s'agit de la musique)
  ret = opus_encoder_ctl(encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_MUSIC));
  if (ret != 0) {
    printf("fail to set signal");
    exit(1);
  }
  return encoder;
}

int encode_audio(OpusEncoder* encoder, const opus_int16* inBuffer, int taille_inBuffer, unsigned char* outBuffer, int taille_outBuffer){

  int ret = opus_encode(encoder, inBuffer, taille_inBuffer, outBuffer, taille_outBuffer);
  if (ret <= 0) {
      printf("fail to decode : %d\n", ret);
      return -1;
  }  

  return 0;
}

void close_encoder(OpusEncoder* encoder){
  free(encoder);
}