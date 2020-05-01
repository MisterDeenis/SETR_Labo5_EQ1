#include <opus/opus.h>

#ifndef COMPRESSION_H
#define COMPRESSION_H

OpusDecoder* decoder_init(int n_channels, int sample_rate);
int decode_audio(OpusDecoder* decoder, const unsigned char* inBuffer, int taille_inBuffer, opus_int16* outBuffer, int taille_outBuffer);
void close_decoder(OpusDecoder* decoder);

OpusEncoder* encoder_init(int n_channels, int sample_rate);
int encode_audio(OpusEncoder* encoder, const opus_int16* inBuffer, int taille_inBuffer, unsigned char* outBuffer, int taille_outBuffer);
void close_encoder(OpusEncoder* encoder);

#endif