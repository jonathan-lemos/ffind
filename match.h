/*
 * Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef __MATCH_H
#define __MATCH_H

#include <regex.h>
#include <pcre.h>
#include <stdint.h>

enum pattern_type{
	TYPE_FNMATCH          = 0,
	TYPE_FNMATCH_ESCAPE   = (1 << 0),
	TYPE_FNMATCH_LITERAL  = (1 << 1),
	TYPE_REGEX_POSIX      = (1 << 2),
	TYPE_REGEX_POSIX_EX   = (1 << 3),
	TYPE_REGEX_PCRE       = (1 << 4),
	TYPE_REGEX_JAVASCRIPT = (1 << 5)
};

#define PFLAG_NORMAL (0)
#define PFLAG_ICASE  (1 << 0)

struct pattern{
	enum pattern_type p_type;
	union pat{
		const char* fnmatch;
		regex_t*    regex;
		pcre*       pcre;
		pcre*       javascript;
	}p;
};

int match(const char* haystack, const struct pattern* needle);
int pat_init(const char* pattern, struct pattern* in_out, unsigned flags);
void pat_free(struct pattern* pat);

#endif
