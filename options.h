/** @file options.h
 * @brief Parses command-line options.
 * @copyright Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef __OPTIONS_H
#define __OPTIONS_H

#include "match.h"
#include <stddef.h>

struct ffind_flags{
	char type;
	unsigned follow_symlink:1;
	unsigned print0:1;
};

struct parsed_data{
	struct ffind_flags flags;
	char** directories;
	size_t directories_len;
	struct pattern pat;
	int maxdepth;
	size_t n_threads;
};

/* Parses options from the command line and stores them in the output variables.
 *
 * Returns 1 if "--help" or "--version" was used, -1 on error, 0 on success.
 */
int parse_options(int argc, char** argv, struct parsed_data* in_out);
void free_options(struct parsed_data* pd);

#endif
