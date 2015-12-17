#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "portmidi.h"
#include "padata.h"
#define STRING_MAX 80 /* used for console input */

/* crash the program to test whether midi ports are closed */

/**/

void doSomethingReallyStupid() {
  int * tmp = NULL;
  *tmp = 5;
}


/* exit the program without any explicit cleanup */

/**/

void doSomethingStupid() {
  assert(0);
}

/* read a number from console */

/**/

int get_number(char *prompt)
{
  char line[STRING_MAX];
  int n = 0, i;
  printf(prompt);
  while (n != 1) {
    n = scanf("%d", &i);
    fgets(line, STRING_MAX, stdin);
  }
  return i;
}

void show_usage()
{
    printf("Usage: test [-h] [-l latency-in-ms]\n");
    exit(0);
}
