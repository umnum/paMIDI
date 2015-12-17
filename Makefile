#makefile for paMIDI
EXE = paMIDI
CC = gcc
LFLAGS = -lportaudio -lportmidi -lm

build: $(EXE)

paMIDI: main.c pafunctions.c pmfunctions.c debug.c midiread.c sine.c padata.h pmdata.h sine.h
	$(CC) -g $? -o $@ $(LFLAGS)

clean:
	rm -f $(EXE)
