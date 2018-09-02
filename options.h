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

/**
 * @brief Parses options from the command line and stores them in the output structure.
 *
 * @param argc The amount of arguments.
 *
 * @param argv The arguments.
 *
 * @param in_out A pointer to a parsed_data structure that will be filled by this function.<br>
 * The data must be freed with free_options() when no longer in use.
 * @see free_options()
 *
 * @return 0 on success, positive if "--help" or "--version" were called, negative on failure.
 */
int parse_options(int argc, char** argv, struct parsed_data* in_out);

/**
 * @brief Frees the data allocated with a previous call to parse_options()
 * @see parse_options()
 *
 * @param pd The parsed data filled with a previous call to parse_options()
 */
void free_options(struct parsed_data* pd);

#endif
