# Copyright (c) 2018 Jonathan Lemos
#
# This software may be modified and distributed under the terms
# of the MIT license.  See the LICENSE file for details.

NAME=ffind
CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99 -pthread
CRELEASEFLAGS=-O2
CDBGFLAGS=-g

FILES=ffind_match
OBJECTS=$(foreach file,$(FILES),$(file).o)
DBGOBJECTS=$(foreach file,$(FILES),$(file).dbg.o)

release: $(OBJECTS) ffind.o
	$(CC) -o $(NAME) ffind.o $(OBJECTS) $(CFLAGS) $(CRELEASEFLAGS)

debug: $(DBGOBJECTS) ffind.dbg.o
	$(CC) -o $(NAME) ffind.dbg.o $(DBGOBJECTS) $(CFLAGS) $(CDBGFLAGS)

test: $(DBGOBJECTS) test.dbg.o
	$(CC) -o test test.dbg.o $(DBGOBJECTS) $(CFLAGS) $(CDBGFLAGS)

%.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(CRELEASEFLAGS)

%.dbg.o: %.c
	$(CC) -c -o $@ $< $(CFLAGS) $(CDBGFLAGS)

.PHONY: clean
clean:
	rm -f $(NAME) $(OBJECTS) $(DBGOBJECTS) test.o test
