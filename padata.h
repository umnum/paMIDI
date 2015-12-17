#include "portaudio.h"
#define NUM_SINEWAVES   (20)  /* total number of sinewaves that
                                 can be active at one time */
#define TABLE_SIZE  (10000)
#define NUM_SECONDS   (5)
#define SAMPLE_RATE   (44100)
#define K_RATE   (4410)
#define FRAMES_PER_BUFFER  (64)
#define ENV_SIZE (44100)
#define HALF_ENV_SIZE (ENV_SIZE/2)
#define MOD_FREQ (40)

typedef struct
{
  float sine[TABLE_SIZE];
  float env[ENV_SIZE];
  float freq, tfreq, vfreq, addfreq;
  float left_phase;
  float right_phase;
  float t_phase, v_phase;
  int note;
  int envcount;
  int velocity;
  int is_note_on;
  char message[20];
}
paTestData;

int patestCallback(const void *,
                   void *,
                   unsigned long,
                   const PaStreamCallbackTimeInfo*,
                   PaStreamCallbackFlags,
                   void *);

void StreamFinished(void *);
