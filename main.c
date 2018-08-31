/*
 * Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "ffind.h"
#include "flags.c"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char** argv){
	pthread_t* threads;
	size_t threads_len;

	if (argc < 3){
		printf("Usage: %s [directory...] [pattern]\n", argv[0]);
		return 1;
	}

	if (ffind_create_threads(argv[1], argv[2], &threads, &threads_len) != 0){
		return 1;
	}
	if (ffind_join_threads(threads, threads_len) != 0){
		return 1;
	}
	return 0;
}
