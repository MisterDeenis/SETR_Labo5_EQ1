/*
 * audio.h
 *
 *  Created on: Apr 12, 2018
 *      Author: Mathieu Garon
 *      Email : mathieu.garon.2@ulaval.ca
 * 
 * Implantation minimaliste d'acquisition/lecture pcm
 */

#ifndef AUDIO_H
#define AUDIO_H

#include <stdio.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>

/* Initialise la structure de donnée
    Params:
        name : nom du device: ex. hw:1,0
        isPlayback : 1 pour le mode lecture, et 0 pour le mode acquisition
        n_channels : nombre de canaux
        frames : nombre d'écahntillon audio
        sample_rate : fréquence d'échantillonnage */
snd_pcm_t* audio_init(const char *name, int isPlayback, int n_channels, snd_pcm_uframes_t *frames, unsigned int *sample_rate);

// Acquisition sur la carte son : frames est le nombre d'échantillons audio
// Retourne un erreur pcm
int audio_read(snd_pcm_t *capture_handle, char *buffer, int frames);
// Écriture sur la carte son : frames est le nombre d'échantillons audio
// Retourne un erreur pcm
int audio_write(snd_pcm_t *capture_handle, char *buffer, int frames);
void audio_destroy(snd_pcm_t *capture_handle);

#endif