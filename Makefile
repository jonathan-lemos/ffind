# Copyright (c) 2018 Jonathan Lemos
#
# This software may be modified and distributed under the terms
# of the MIT license.  See the LICENSE file for details.

NAME=ffind
CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99 -pthread -D_XOPEN_SOURCE=500
LDFLAGS=-lpcre
CRELEASEFLAGS=-O2
CDBGFLAGS=-g

FILES=match ffind options log
OBJECTS=$(foreach file,$(FILES),$(file).o)
DBGOBJECTS=$(foreach file,$(FILES),$(file).dbg.o)

release: $(OBJECTS) main.o
	$(CC) -o $(NAME) main.o $(OBJECTS) $(CFLAGS) $(CRELEASEFLAGS) $(LDFLAGS)

debug: $(DBGOBJECTS) main.dbg.o
	$(CC) -o $(NAME) main.dbg.o $(DBGOBJECTS) $(CFLAGS) $(CDBGFLAGS) $(LDFLAGS)

test: $(DBGOBJECTS) test.dbg.o
	$(CC) -o test test.dbg.o $(DBGOBJECTS) $(CFLAGS) $(CDBGFLAGS) $(LDFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(CRELEASEFLAGS)

%.dbg.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(CDBGFLAGS)

.PHONY: clean
clean:
	rm -f $(NAME) $(OBJECTS) $(DBGOBJECTS) test.dbg.o test main.dbg.o main.o
