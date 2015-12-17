#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "sine.h"

/* set the sinewave frequency */
void setFreq(void *sw, float freq) {
  sinewave *sinw = (sinewave*)sw;
  sinw->data.freq = freq;
  sinw->delI = (freq * TABLE_SIZE) / (float)SAMPLE_RATE;
}

/* add to the sinewave base frequency,
   used for modifying frequency with vibrato */
void addFreq(void *sw, float freq) {
  sinewave *sinw = (sinewave*)sw;
  sinw->data.addfreq = freq;
  sinw->delAddI = (freq * TABLE_SIZE) / (float)SAMPLE_RATE;
}

/* set the tremolo frequency */
void setTremFreq(void *sw, float freq) {
  sinewave *sinw = (sinewave*)sw;
  sinw->data.tfreq = freq;
  sinw->delTremI = (freq * TABLE_SIZE) / (float)SAMPLE_RATE;
}

/* set the vibrato frequency */
void setVibFreq(void *sw, float freq) {
  sinewave *sinw = (sinewave*)sw;
  sinw->data.vfreq = freq;
  sinw->delVibI = (freq * TABLE_SIZE) / (float)SAMPLE_RATE;
}

/* set the sinewave amplitude */
void setAmp(void *sw, float amp) {
  sinewave *sinw = (sinewave*)sw;
  sinw->amplitude = amp;
}

/* set the tremolo amplitude */
void setTremAmp(void *sw, float amp) {
  sinewave *sinw = (sinewave*)sw;
  sinw->tamplitude = amp;
}

/* set the vibrato amplitude */
void setVibAmp(void *sw, float amp) {
  sinewave *sinw = (sinewave*)sw;
  sinw->vamplitude = amp;
}

/* set the envelope for a sinewave */
void setEnv(void *sw) {
  int i, env_size, half_env_size;
  float slope, amp;
  sinewave *sinw = (sinewave*)sw;
  amp = sinw->amplitude;
  env_size = sinw->env_size;
  half_env_size = env_size/2;
  slope = amp/half_env_size;
  for(i = 0; i < half_env_size; i++) {
    sinw->data.env[i] = slope * i;
  }
  slope *= -1;
  for(i = half_env_size; i < env_size; i++) {
    sinw->data.env[i] = slope * (i - half_env_size) + amp;
  }
}

/* grab the current envelope value and
   update the next value */
float envTick(void *sw) {
  float amp, v_phase, t_phase;
  int count, is_note_on, env_size, half_env_size;
  sinewave *sinw = (sinewave*)sw;

  amp = sinw->amplitude;
  count = sinw->data.envcount;
  is_note_on = sinw->data.velocity;
  env_size = sinw->env_size;
  half_env_size = env_size/2;
  v_phase = sinw->data.v_phase;
  t_phase = sinw->data.t_phase;

  if (abs(v_phase - 3*TABLE_SIZE/4) < 5) {
    sinw->is_sustain_on = 0;
  }
  if (t_phase < 5) {
    sinw->is_sustain_on = 0;
  }
  if (count >= env_size) {
    sinw->data.envcount = 0;
    return 0;
  }
  if (count == half_env_size && is_note_on) {
    sinw->is_sustain_on = 1;
    return amp;
  }
  if (!is_note_on && !count) {
    sinw->data.is_note_on = 0;
    return 0;
  }

  amp = sinw->data.env[count];
  sinw->data.envcount++;
  return amp;
}

/* grab the current sinewave value and
   update the next value */
float swTick(void *sw) {
  float amp;
  sinewave *sinw = (sinewave*)sw;
  amp = sinw->data.sine[(int)sinw->data.right_phase];
  sinw->data.left_phase += sinw->delI + sinw->delAddI;
  while(sinw->data.left_phase >= TABLE_SIZE) sinw->data.left_phase -= TABLE_SIZE;
  sinw->data.right_phase += sinw->delI + sinw->delAddI;
  while(sinw->data.right_phase >= TABLE_SIZE) sinw->data.right_phase -= TABLE_SIZE;
  if(sinw->kcount<0) sinw->kcount=0;
  sinw->krate = sinw->kcount++ % (SAMPLE_RATE/K_RATE) == 0;
  return amp;
}

/* grab the current tremolo value and 
   update the next value */
float tremTick(void *sw) {
  float freq;
  sinewave *sinw = (sinewave*)sw;
  freq = sinw->data.sine[(int)sinw->data.t_phase];
  sinw->data.t_phase += sinw->delTremI*((double)SAMPLE_RATE/(double)K_RATE) ;
  while(sinw->data.t_phase >= TABLE_SIZE) sinw->data.t_phase -= TABLE_SIZE;
  return freq;
}

/* grab the current vibrato value and 
   update the next value */
float vibTick(void *sw) {
  float amp;
  sinewave *sinw = (sinewave*)sw;
  amp = sinw->data.sine[(int)sinw->data.v_phase];
  sinw->data.v_phase += sinw->delVibI*((double)SAMPLE_RATE/(double)K_RATE) ;
  while(sinw->data.v_phase >= TABLE_SIZE) sinw->data.v_phase -= TABLE_SIZE;
  return amp;
}

/* initialize a sinewave */
void swInit(void *sw) {

  int i;
  /* initialize sinewave table */
  sinewave *sinw = (sinewave*)sw;
  for(i=0; i<TABLE_SIZE; i++) {
    sinw->data.sine[i] = (float) sin(((double)i/(double)TABLE_SIZE) * M_PI * 2.);
  }

  /* initialize the sinewave, tremolo, and vibrato phase */
  sinw->data.left_phase = sinw->data.right_phase = 0;
  sinw->data.t_phase = 3*TABLE_SIZE/4;
  sinw->data.v_phase = 0;
  
  /* initialize envelope counter */
  sinw->data.envcount = 0;

  /* create a default amplitude envelope size */
  sinw->env_size = 5000;

  /* initialize krate values */
  sinw->kcount = 0;
  sinw->krate = 0;
  sinw->kamp = 1;
  sinw->kfreq = 0;

  /* initialize sinewave frequency */
  setFreq(sw, 0);

  /* tremolo is off by default */
  sinw->is_sustain_on = 0;
  setTremFreq(sw, 0);
  setTremAmp(sw, 0);

  /* vibrato is off by default */
  setVibFreq(sw, 0);
  addFreq(sw, 0);
  setVibAmp(sw, 0);

  /* sinewave functions can be accessed 
     within a sinewave variable */
  sinw->tick = &swTick;
  sinw->envtick = &envTick;
  sinw->tremtick = &tremTick;
  sinw->vibtick = &vibTick;
  sinw->freq = &setFreq;
  sinw->tfreq = &setTremFreq;
  sinw->vfreq = &setVibFreq;
  sinw->addfreq = &addFreq;
  sinw->amp = &setAmp;
  sinw->tamp = &setTremAmp;
  sinw->vamp = &setVibAmp;
  sinw->env = &setEnv;
}
