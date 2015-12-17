#include "porttime.h"
#include "pmdata.h"
#include "stdlib.h"
#include "stdio.h"
#include "assert.h"
#include "string.h"
#include "math.h"
#include "sine.h"

#define INPUT_BUFFER_SIZE 100
#define OUTPUT_BUFFER_SIZE 0
#define DRIVER_INFO NULL
#define TIME_PROC Pt_Time
#define TIME_INFO NULL
#define TIME_START Pt_Start(1, 0, 0) /* timer started w/millisecond accuracy */
#define STRING_MAX 80 /* used for console input */

/* read MIDI values and process audio */

int midi_read()
{
  int i = 0, n = 0, j;
  int in, out;
  int midi_note;
  int note_velocity;
  int is_note_on;
  int note_status;
  long latency = 5; // 5ms latency
  long freq;
  PmStream * midi;
  PmEvent bufferIn[1]; // process one MIDI event at a time
  PmError status, length;

  PaStreamParameters outputParameters;
  PaStream *stream;
  PaError err;
  sinewave sw[NUM_SINEWAVES];

  /* table of midi values from midi note 37 to 48 */
                            // C#      D       D#      E       F       F#      G       G#      A       A#      B       C
  float midiToFrequency[12] = {69.296, 73.416, 77.782, 82.407, 87.307, 92.499, 97.999, 103.83, 110.00, 116.54, 123.47, 130.81};

  /** open portaudio output stream **/
  printf("PortAudio Test: output sine wave. SR = %d, BufSize = %d\n", SAMPLE_RATE, FRAMES_PER_BUFFER);

  /* initialise sinusoidal wavetable */
  for (i = 0; i < NUM_SINEWAVES; i++) {
    swInit(&sw[i]);
    sw[i].data.velocity = 0;
  }


  /** start portaudio **/

  err = Pa_Initialize();
  if( err != paNoError ) return err;

  outputParameters.device = Pa_GetDefaultOutputDevice(); /* default output device */
  if (outputParameters.device == paNoDevice) {
    fprintf(stderr,"Error: No default output device.\n");
    return err;
  }   
  outputParameters.channelCount = 2;       /* stereo output */
  outputParameters.sampleFormat = paFloat32; /* 32 bit floating point output */
  outputParameters.suggestedLatency = Pa_GetDeviceInfo( outputParameters.device )->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;

  err = Pa_OpenStream(
                      &stream,
                      NULL, /* no input */
                      &outputParameters,
                      SAMPLE_RATE,
                      FRAMES_PER_BUFFER,
                     paClipOff,      /* we won't output out of range samples so don't bother clipping them */
                      patestCallback,
                      &sw);
  if( err != paNoError ) return err;

  for (i = 0; i < NUM_SINEWAVES; i++) {
    sprintf( sw[0].data.message, "No Message" );
  }
  err = Pa_SetStreamFinishedCallback( stream, &StreamFinished );
  if( err != paNoError ) return err;

  err = Pa_StartStream( stream );
  if( err != paNoError ) return err;


  /** get portmidi input device **/

  in = get_number("Type input number: ");

  /* In is recommended to start timer before PortMidi */
  TIME_START;

  /* open input device */
  Debug(Pm_OpenInput(&midi, 
                     in,
                     DRIVER_INFO, 
                     INPUT_BUFFER_SIZE, 
                     TIME_PROC, 
                     TIME_INFO
                     )); /* midi thru */

  printf("Midi Input opened. Reading Midi messages... (press MIDI pitch 36 to quit)\n");

  Pm_SetFilter(midi, PM_FILT_ACTIVE | PM_FILT_CLOCK);

  /* empty the buffer after setting filter, just in case anything
     got through */
  /*while (Pm_Poll(midi)) {
    DebugStream(Pm_Read(midi, bufferIn, 1), midi);
  }*/
  for (i=0; i < 5; i++) {
    while (Pm_Poll(midi)) {
      DebugStream(Pm_Read(midi, bufferIn, 1), midi);
    }  
    DebugStream(Pm_Read(midi, bufferIn, 1), midi);
  }

  i = 0;

  while (1) {

    DebugStream((status = Pm_Poll(midi)),midi);

    /** process a MIDI event, if there is no MIDI to process
        then put the main thread to sleep **/
    if (status == TRUE) {

      // read MIDI note
      DebugStream((length = Pm_Read(midi,bufferIn,1)),midi);

      // get MIDI values
      midi_note = Pm_MessageData1(bufferIn[0].message);
      note_velocity = Pm_MessageData2(bufferIn[0].message);
      note_status = Pm_MessageStatus(bufferIn[0].message);
      is_note_on = note_velocity;

      /* find the jth sinewave that can be activated
         by the current MIDI event */

      if (note_status != 176) { // not a control event
        if (note_status = 144) { // a key is pressed
          for (j = 0; j < NUM_SINEWAVES; j++) {
            if (sw[j].data.is_note_on && (sw[j].data.note == midi_note)) {
              /* trying to trigger a note that hasn't fully released
                 release note so that it can be triggered again */
              sw[j].data.velocity = 0;
            }
          }
          for (j = 0; j < NUM_SINEWAVES; j++) {
            // find the first sinewave that isn't active
            if (!sw[j].data.is_note_on) {
              break;
            }
          }
          if (j == NUM_SINEWAVES) {
            /* at this point all sinewaves are active,
               since there is no room, the current MIDI
               event will overwrite the last sinewave */
            j = 19;
          }
        }
        if (!is_note_on) {
          for (j = 0; j < NUM_SINEWAVES; j++) {
            /* make sure the MIDI note off event
               will turn off the right sinewave */
            if (sw[j].data.note == midi_note) {
              break;
            }
          }
        }
        else {
          // jth sinewave is now active
          sw[j].data.is_note_on = 1;
        }
      }

      /** special MIDI events **/

      if (midi_note == 36 ) // exit program
        break;

      /** MIDI controller events **/
      if (note_status == 176 && midi_note == 74) { // change envelope size
        for (j = 0; j < NUM_SINEWAVES; j++) {
          sw[j].env_size = (int)(note_velocity * (ENV_SIZE-1) / 127 + 1);
        }
        continue;
      }
      if (note_status == 176 && midi_note == 93) { // change vibrato frequency
        for (j = 0; j < NUM_SINEWAVES; j++) {
          (sw[j].vfreq)(&sw[j], (float)(note_velocity * MOD_FREQ) / 127.0);
        }
        continue;
      }
      if (note_status == 176 && midi_note == 73) { // change vibrato amplitude
        for (j = 0; j < NUM_SINEWAVES; j++) {
          (sw[j].vamp)(&sw[j], (float)(note_velocity) / 127.0);
        }
        continue;
      }
      if (note_status == 176 && midi_note == 71) { // change tremolo frequency
        for (j = 0; j < NUM_SINEWAVES; j++) {
          (sw[j].tfreq)(&sw[j], (float)(note_velocity * MOD_FREQ) / 127.0);
        }
        continue;
      }
      if (note_status == 176 && midi_note == 91) { // change tremolo amplitude
        for (j = 0; j < NUM_SINEWAVES; j++) {
          (sw[j].tamp)(&sw[j], (float)(note_velocity) / 127.0);
        }
        continue;
      }
      // ignore all other MIDI controller events
      if (note_status == 176) { 
        continue;
      }


      /* a MIDI note-on event will turn on sound in the callback function */
      sw[j].data.velocity = note_velocity;
      sw[j].data.note = midi_note;

      // set sinewave amplitude
      (sw[j].amp)(&sw[j], (float)(note_velocity)/127.0);

      // set sinewave frequency
      freq = midiToFrequency[(midi_note-1) % 12];
      
      // adjust the note octave
      if (midi_note > 84 && midi_note < 97) {
        freq *= 16;
      } else if (midi_note > 72) {
        freq *= 8;
      } else if (midi_note > 60) {
        freq *= 4;
      } else if (midi_note > 48) {
        freq *= 2;
      } else if (midi_note > 36) {
      } else {
        freq *= 0; // outside the range
      }
      (sw[j].freq)(&sw[j], freq);

      // create amplitude envelope for sinewave
      if (is_note_on) {
        (sw[j].env)(&sw[j]);
      }

      if (length > 0) {
 
          printf("Input message %d: time %d, %d %d %d\n",
                 i,
                 bufferIn[0].timestamp,
                 Pm_MessageStatus(bufferIn[0].message),
                 Pm_MessageData1(bufferIn[0].message),
                 Pm_MessageData2(bufferIn[0].message));
 
          i++;
 
      } else {
        assert(0);
      }
    } else {
      // put main thread to sleep for 10ms
      Pa_Sleep(10);
    }
  }

  err = Pa_StopStream( stream );

  if( err != paNoError ) {
    return err;
  }
  else {
    err = Pa_CloseStream( stream );
    return err;
  }

  /* since close device should not needed, lets get
     rid of it just to make sure program exit closes MIDI devices */

  /* Pm_Close(midi);
    Pm_Close(midiOut);
    Pm_Terminate(); */
}
