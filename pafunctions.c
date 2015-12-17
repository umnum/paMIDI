#include <stdio.h>
#include "portaudio.h"
#include "sine.h"

/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/
int patestCallback( const void *inputBuffer, void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData )
{
    sinewave *sw = (sinewave*)userData; // pass through data for all sinewaves
    float *out = (float*)outputBuffer;
    unsigned long i;
    float amp = 0;
    float freq;
    int j;

    (void) timeInfo; /* Prevent unused variable warnings. */
    (void) statusFlags;
    (void) inputBuffer;
 
    // write sine values to output buffer
    for( i=0; i<framesPerBuffer; i++)
    {
      amp = 0;
      // add vibrato and tremolo at krate
      for (j=0; j<NUM_SINEWAVES; j++) {
        if ((&sw[j])->is_sustain_on && (&sw[j])->krate) { 
          // tremolo starts at 0 (kfreq = 0)
          (&sw[j])->kamp = 1 - (((&sw[j])->tremtick)(&sw[j]) + 1)*0.5*(&sw[j])->tamplitude;
          // vibrato starts at peak amplitude (kamp = 1)
          (&sw[j])->kfreq = (((&sw[j])->vibtick)(&sw[j]) + 1)*0.5*MOD_FREQ*(&sw[j])->vamplitude;
        }
        // multiply amplitude envelope and tremolo with sine wave
        amp += ((&sw[j])->tick)(&sw[j]) * ((&sw[j])->envtick)(&sw[j]) * (&sw[j])->kamp/(j+1);
      }
      *out++ = amp;
      *out++ = amp;
    }   

    for (j=0; j<NUM_SINEWAVES; j++) {
      // add vibrato when sustain is on
      if ((&sw[j])->is_sustain_on) {
        ((&sw[j])->addfreq)(&sw[j], (&sw[j])->kfreq);
      } else {
        ((&sw[j])->addfreq)(&sw[j], 0);
      }
    }

    if ((&sw[0])->data.note == 36)
      // program is finished, finish callback and exit
      return paComplete;
    else
      return paContinue;
}

/*
 * This routine is called by portaudio when playback is done.
 */
void StreamFinished( void* userData )
{
  paTestData *data = (paTestData *) userData;
  printf( "Stream Completed: %s\n", data->message );
}
