# Copyright (c) 2018 Jonathan Lemos
#
# This software may be modified and distributed under the terms
# of the MIT license.  See the LICENSE file for details.

NAME=ffind
CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99 -pthread
CRELEASEFLAGS=-O2
CDBGFLAGS=-g -fsanitize=address -fno-omit-frame-pointer

release: $(NAME)
	$(CC) ffind.c -o $(NAME) $(CFLAGS) $(CRELEASEFLAGS)

debug: $(NAME)
	$(CC) ffind.c -o $(NAME) $(CFLAGS) $(CDBGFLAGS)

.PHONY: clean
clean:
	rm -f $(NAME)
