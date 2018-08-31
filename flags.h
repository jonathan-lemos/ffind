/*
 * Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef __FLAGS_H
#define __FLAGS_H

#include "match.h"
#include <stddef.h>

struct ffind_flags{
	char type;
	unsigned follow_symlink:1;
	unsigned print0:1;
};

/* Parses options from the command line and stores them in the output variables.
 *
 * Returns 1 if "--help" or "--version" was used, -1 on error, 0 on success.
 */
int parse_options(int argc, char** argv, struct ffind_flags* flags_out, char*** dir_out, size_t* dir_out_len, struct pattern* pattern_out);

#endif
