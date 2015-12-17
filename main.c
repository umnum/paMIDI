#include "porttime.h"
#include "padata.h"
#include "pmdata.h"
#include "stdlib.h"
#include "stdio.h"
#include "assert.h"
#include "string.h"
#include "math.h"

#define INPUT_BUFFER_SIZE 100
#define OUTPUT_BUFFER_SIZE 0
#define DRIVER_INFO NULL
#define TIME_PROC Pt_Time
#define TIME_INFO NULL
#define TIME_START Pt_Start(1, 0, 0) /* timer started w/millisecond accuracy */
#define STRING_MAX 80 /* used for console input */

long latency = 0;

/*
 * the somethingStupid parameter can be set to simulate a program crash.
 * We want PortMidi to close Midi ports automatically in the event of a
 * crash because Windows does not (and this may cause an OS crash)
 */

int main(int argc, char *argv[])
{
  int i = 0, n = 0;
  char line[STRING_MAX];
  char ch;
  int test_input = 0, test_output = 0, test_both = 1, somethingStupid = 0;
  int stream_test = 0;
  int latency_valid = TRUE;
  int err = 0;
  char buffer[STRING_MAX];
  latency = 5;

  /* list device information */
  for (i = 0; i < Pm_CountDevices(); i++) {
    const PmDeviceInfo *info = Pm_GetDeviceInfo(i);
    if (((test_input  | test_both) & info->input) |
        ((test_output | test_both | stream_test) & info->output)) {
        printf("%d: %s, %s", i, info->interf, info->name);
        if (info->input) printf(" (input)");
        if (info->output) printf(" (output)");
        printf("\n");
    }
  }
    
  /* run test */
  err = midi_read();

  if ( err != paNoError ) goto error;

  Pa_Terminate();
  printf("finished portaudio test...\n");
  printf("finished portMidi test...type ENTER to quit...");
  fgets(line, STRING_MAX, stdin);

  return err;

	error: 
	Pa_Terminate();
	fprintf( stderr, "An error occured while using the portaudio stream\n" );
	fprintf( stderr, "Error number: %d\n", err );
	fprintf( stderr, "Error message: %s\n", Pa_GetErrorText( err ) );
  printf("finished portMidi test...type ENTER to quit...");
  fgets(line, STRING_MAX, stdin);

  return err;
}
