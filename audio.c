#include "audio.h"

snd_pcm_t* audio_init(const char *name, int isPlayback, int n_channels, snd_pcm_uframes_t *frames, unsigned int *sample_rate)
{
  int err;
  snd_pcm_hw_params_t *hw_params;
  snd_pcm_t *capture_handle;
  snd_pcm_stream_t stream_type = SND_PCM_STREAM_CAPTURE;
  if(isPlayback)
  {
	  stream_type = SND_PCM_STREAM_PLAYBACK;
  }

  // Ouverture et enregistrement des paramètres pcm
  if ((err = snd_pcm_open(&capture_handle, name, stream_type, 0)) < 0) {
    fprintf (stderr, "cannot open audio device %s (%s)\n",
    		 name,
             snd_strerror (err));
    return NULL;
  }

  fprintf(stdout, "Audio interface : %s\nPlayback : %d\n", name, isPlayback);

  if ((err = snd_pcm_hw_params_malloc (&hw_params)) < 0) {
    fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
             snd_strerror (err));
    return NULL;
  }


  if ((err = snd_pcm_hw_params_any (capture_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot initialize hardware parameter structure (%s)\n",
             snd_strerror (err));
    return NULL;
  }


  if ((err = snd_pcm_hw_params_set_access (capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    fprintf (stderr, "cannot set access type (%s)\n",
             snd_strerror (err));
    return NULL;
  }

  if ((err = snd_pcm_hw_params_set_format (capture_handle, hw_params, SND_PCM_FORMAT_S16_LE)) < 0) {
    fprintf (stderr, "cannot set sample format (%s)\n",
             snd_strerror (err));
    return NULL;
  }

  if ((err = snd_pcm_hw_params_set_rate_near (capture_handle, hw_params, sample_rate, 0)) < 0) {
    fprintf (stderr, "cannot set sample rate (%s)\n",
             snd_strerror (err));
    return NULL;
  }


  if ((err = snd_pcm_hw_params_set_channels (capture_handle, hw_params, n_channels)) < 0) {
    fprintf (stderr, "cannot set channel count (%s)\n",
             snd_strerror (err));
    return NULL;
  }

  fprintf(stdout, "hw_params channels : %d\n", n_channels);

  if(*frames != 0){
    if ((err = snd_pcm_hw_params_set_period_size_near(capture_handle, hw_params, frames, 0)) < 0) {
      fprintf (stderr, "cannot set period size (%s)\n",
              snd_strerror (err));
      return NULL;
    }
  }
  
  if ((err = snd_pcm_hw_params (capture_handle, hw_params)) < 0) {
    fprintf (stderr, "cannot set parameters (%s)\n",
             snd_strerror (err));
    return NULL;
  }

  snd_pcm_hw_params_free (hw_params);

  fprintf(stdout, "hw_params period : %u\n", (unsigned int)*frames);
  fprintf(stdout, "hw_params rate : %d\n", *sample_rate);

  if ((err = snd_pcm_prepare (capture_handle)) < 0) {
    fprintf (stderr, "cannot prepare audio interface for use (%s)\n",
             snd_strerror (err));
    return NULL;
  }

  return capture_handle;
}

int audio_read(snd_pcm_t *capture_handle, char *buffer, int frames){
	int err;

	err = snd_pcm_readi(capture_handle, buffer, frames);
  if (err == -EPIPE) {
    /* EPIPE means underrun */
    fprintf(stderr, "underrun occurred\n");
    snd_pcm_prepare(capture_handle);
  } else if (err < 0) {
    fprintf(stderr, "error from readi: %s\n", snd_strerror(err));
  }  else if (err != (int)frames) {
    fprintf(stderr, "short read, read %d frames\n", err);
  }
	return err;
}

int audio_write(snd_pcm_t *capture_handle, char *buffer, int frames){
	int err;

	err = snd_pcm_writei(capture_handle, buffer, frames);
  if (err == -EPIPE) {
    /* EPIPE means underrun */
    fprintf(stderr, "underrun occurred\n");
    snd_pcm_prepare(capture_handle);
    err = 0;
  } else if (err < 0) {
    fprintf(stderr, "error from writei: %s\n", snd_strerror(err));
  }  else if (err != (int)frames) {
    fprintf(stderr, "short write, write %d frames\n", err);
  }
	return err;
}

void audio_destroy(snd_pcm_t *capture_handle){
	snd_pcm_close(capture_handle);
	fprintf(stdout, "audio interface closed\n");
}
