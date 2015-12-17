#include "padata.h"

typedef struct {

  paTestData data;
  float delI, delTremI, delVibI, delAddI;
  float amplitude, tamplitude, vamplitude, kamp, kfreq;
  int krate, kcount;
  int env_size;
  int is_sustain_on;

  /* sinewave functions called within
     the sinewave variable */
  float (*tick)(void *self);
  float (*tremtick)(void *self);
  float (*vibtick)(void *self);
  float (*envtick)(void *self);
  void (*freq)(void *self, float frequency);
  void (*tfreq)(void *self, float frequency);
  void (*vfreq)(void *self, float frequency);
  void (*addfreq)(void *self, float frequency);
  void (*amp)(void *self, float amplitude);
  void (*tamp)(void *self, float amplitude);
  void (*vamp)(void *self, float amplitude);
  void (*env)(void *self);
} sinewave;

void swInit(void *);
