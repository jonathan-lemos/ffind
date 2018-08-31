/*
 * Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "log.h"
#include "match.h"
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <fnmatch.h>

int match_fnmatch(const char* haystack, const char* needle){
	return fnmatch(needle, haystack, 0);
}

int match_fnmatch_literal(const char* haystack, const char* needle){
	return strstr(haystack, needle) != NULL;
}

/* Returns true if the needle is within the haystack, false if not.
 *
 * The needle can include the '*' character, which matches 0 or more of any character.
 * A literal '*' can be inputted with "\*"
 */
int match_fnmatch_escape(const char* haystack, const char* needle){
	size_t haystack_len = strlen(haystack);
	size_t needle_len = strlen(needle);
	size_t haystack_ptr = 0;
	size_t needle_ptr = 0;
	for (; haystack_ptr <= haystack_len && needle_ptr <= needle_len; haystack_ptr++, needle_ptr++){
		if (needle[needle_ptr] == '\\'){
			needle_ptr++;
		}
		else if (needle[needle_ptr] == '*'){
			needle_ptr++;
			while (haystack[haystack_ptr] != '\0' && haystack[haystack_ptr] != needle[needle_ptr]){
				haystack_ptr++;
			}
		}

		if (haystack[haystack_ptr] != needle[needle_ptr]){
			return 0;
		}
	}
	haystack_ptr--;
	needle_ptr--;
	return haystack[haystack_ptr] == '\0' && needle[needle_ptr] == '\0';
}

int match_regex_posix(const char* haystack, const regex_t* regex){
	int res = regexec(regex, haystack, 0, NULL, 0);
	return res == 0;
}

int match_regex_pcre(const char* haystack, const pcre* pcre){
	int res = pcre_exec(pcre, NULL, haystack, strlen(haystack), 0, 0, NULL, 0);
	return !(res == PCRE_ERROR_NOMATCH);
}

#define match_regex_javascript(haystack, needle) match_regex_pcre(haystack, needle)

int match(const char* haystack, struct pattern* needle){
	switch (needle->p_type){
	case TYPE_FNMATCH:
		return match_fnmatch(haystack, needle->p.fnmatch);
	case TYPE_FNMATCH_ESCAPE:
		return match_fnmatch_escape(haystack, needle->p.fnmatch);
	case TYPE_FNMATCH_LITERAL:
		return match_fnmatch_literal(haystack, needle->p.fnmatch);
	case TYPE_REGEX_POSIX_EX:
	case TYPE_REGEX_POSIX:
		return match_regex_posix(haystack, needle->p.regex);
	case TYPE_REGEX_PCRE:
		return match_regex_pcre(haystack, needle->p.pcre);
	case TYPE_REGEX_JAVASCRIPT:
		return match_regex_pcre(haystack, needle->p.javascript);
	}
}

int pat_create(const char* pattern, enum pattern_type p_type, uint_fast8_t flags, union pat* out){
	int res;
	const char* err;

	switch (p_type){
	case TYPE_FNMATCH:
	case TYPE_FNMATCH_ESCAPE:
	case TYPE_FNMATCH_LITERAL:
		out->fnmatch = pattern;
		return 0;

	case TYPE_REGEX_POSIX_EX:
		flags |= REG_EXTENDED;
		//fall through
	case TYPE_REGEX_POSIX:
		res = regcomp(out->regex, pattern, flags);
		if (res != 0){
			char errbuf[256];
			regerror(res, out->regex, errbuf, sizeof(errbuf));
			eprintf_mt("ffind: Failed to create posix regex (%s)\n", errbuf);
			return -1;
		}
		return 0;

	case TYPE_REGEX_PCRE:
		out->pcre = pcre_compile(pattern, flags, &err, &res, NULL);
		if (!out->pcre){
			eprintf_mt("ffind: Failed to compile pcre regex at character %d (%s)\n", res, err);
			return -1;
		}
		return 0;

	case TYPE_REGEX_JAVASCRIPT:
		out->javascript = pcre_compile(pattern, flags, &err, &res, NULL);

		if (!out->javascript){
			eprintf_mt("ffind: Failed to compile javascript regex at character %d (%s)\n", res, err);
			return -1;
		}
		return 0;
	}
}

void pat_free(enum pattern_type p_type, union pat* p){
	switch (p_type){
		case TYPE_FNMATCH:
		case TYPE_FNMATCH_ESCAPE:
		case TYPE_FNMATCH_LITERAL:
			return;
		case TYPE_REGEX_POSIX:
		case TYPE_REGEX_POSIX_EX:
			regfree(p->regex);
			return;
		case TYPE_REGEX_PCRE:
			pcre_free(p->pcre);
			return;
		case TYPE_REGEX_JAVASCRIPT:
			pcre_free(p->javascript);
			return;
	}
}
