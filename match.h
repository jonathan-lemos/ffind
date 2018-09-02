/** @file match.h
 * @brief Pattern matching backend.
 * @copyright Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef __MATCH_H
#define __MATCH_H

#include "attribute.h"
#include <regex.h>
#include <pcre.h>
#include <stdint.h>

/**
 * @brief The type of pattern to use.
 */
enum pattern_type{
	TYPE_FNMATCH          = 0,        /**< The default. Match using the fnmatch(3) function. */
	TYPE_FNMATCH_ESCAPE   = (1 << 0), /**< Match using a similar function to fnmatch(3) that supports escaping special characters. */
	TYPE_FNMATCH_LITERAL  = (1 << 1), /**< Match using strstr(3). */
	TYPE_REGEX_POSIX      = (1 << 2), /**< Match using POSIX basic regular expressions. */
	TYPE_REGEX_POSIX_EX   = (1 << 3), /**< Match using POSIX extended regular expressions. */
	TYPE_REGEX_PCRE       = (1 << 4), /**< Match using Perl-compatible regular expressions (PCRE). */
	TYPE_REGEX_JAVASCRIPT = (1 << 5)  /**< Match using Javascript regular expressions. */
};

#define PFLAG_NORMAL (0)      /**< No special flags. Only valid by itself. */
#define PFLAG_ICASE  (1 << 0) /**< Ignore case when searching. */

/**
 * @brief A pattern to match.
 */
struct pattern{
	enum pattern_type p_type;   /**< The type of pattern to use. */
	union pat{                  /**< A pattern object corresponding to the p_type. */
		const char* fnmatch;
		regex_t*    regex;
		pcre*       pcre;
		pcre*       javascript;
	}p;
};

/**
 * @brief Checks to see if the needle is within the haystack.
 *
 * @param haystack A string to search within.
 *
 * @param needle A pattern to search the haystack for.<br>
 * Use pat_init() to create a pattern.
 *
 * @return True for a match, false for no match.
 */
int match(const char* haystack, const struct pattern* needle) FF_HOT FF_INLINE;

/**
 * @brief Initializes a pattern structure.
 *
 * @param pattern The text to create the pattern out of.
 *
 * @param in_out The pattern object to fill.<br>
 * in_out->p_type should be filled in with the type of pattern to create before this function is called.<br>
 * in_out->p will be filled with the generated pattern.<br>
 * This pattern must be freed when no longer in use with pat_free()
 * @see pat_free()
 *
 * @param flags Special flags to use.<br>
 * These can be combined with the '|' operator.
 * @see PFLAG_ICASE
 *
 * @return 0 on success, negative on failure.
 */
int pat_init(const char* pattern, struct pattern* in_out, unsigned flags);

/**
 * @brief Frees the pattern within a pattern structure.
 *
 * @param pat The pattern structure to free.
 */
void pat_free(struct pattern* pat);

#endif
