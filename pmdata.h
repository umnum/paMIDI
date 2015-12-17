#include "portmidi.h"

void doSomethingReallyStupid();

void doSomethingStupid();

int get_number(char *);

static void prompt_and_exit(void);

void Debug(PmError);

void DebugStream(PmError, PortMidiStream *);

void show_usage();

int midi_read();
