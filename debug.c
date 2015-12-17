#include <stdio.h>
#include <stdlib.h>
#include "portmidi.h"
#define STRING_MAX 80 /* used for console input */

/*
=====================================================================
routines for client debugging
=====================================================================
    this stuff really important for debugging client app. These are
    not included in PortMidi because they rely on console and printf()
*/


static void prompt_and_exit(void)
{
  char line[STRING_MAX];
  printf("type ENTER...");
  fgets(line, STRING_MAX, stdin);
  /* this will clean up open ports: */
  exit(-1);
}


void Debug(PmError error)
{
  /* note that errors are negative and some routines return
   * positive values to indicate success status rather than error
   */
  if (error < 0) {
    printf("PortMidi call failed...\n");
    printf(Pm_GetErrorText(error));
    prompt_and_exit();
  }
}

void DebugStream(PmError error, PortMidiStream * stream) {
  if (error == pmHostError) {
    char msg[PM_HOST_ERROR_MSG_LEN];
    printf("HostError: ");
    /* this function handles bogus stream pointer */
    Pm_GetHostErrorText(msg, PM_HOST_ERROR_MSG_LEN);
    printf("%s\n", msg);
    prompt_and_exit();
  } else if (error < 0) {
    Debug(error);
  }
}
