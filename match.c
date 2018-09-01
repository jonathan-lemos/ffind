/** @file match.h
 * @brief Pattern matching backend.
 * @copyright Copyright (c) 2018 Jonathan Lemos
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
	return fnmatch(needle, haystack, 0) == 0;
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
	return res != PCRE_ERROR_NOMATCH;
}

#define match_regex_javascript(haystack, needle) match_regex_pcre(haystack, needle)

__attribute__((unused))
static char* convert_tolcase(const char* s){
	char* str = malloc(strlen(s) + 1);
	if (!str){
		log_enomem();
		return NULL;
	}
	for (size_t i = 0; i < strlen(s) + 1; ++i){
		char c = s[i];
		if (c >= 'A' && c <= 'Z'){
			c += 'a' - 'A';
		}
		str[i] = c;
	}
	return str;
}

int match(const char* haystack, const struct pattern* needle){
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
	return 0;
}

unsigned flags_convert(enum pattern_type p_type, unsigned f){
	int flags_new = 0;

	switch (p_type){
	case TYPE_FNMATCH:
	case TYPE_FNMATCH_ESCAPE:
	case TYPE_FNMATCH_LITERAL:
		flags_new |= PFLAG_ICASE;
		break;

	case TYPE_REGEX_POSIX_EX:
		flags_new |= REG_EXTENDED;
		//fall through
	case TYPE_REGEX_POSIX:
		if (f & PFLAG_ICASE){
			flags_new |= REG_ICASE;
		}
		break;

	case TYPE_REGEX_JAVASCRIPT:
		flags_new |= PCRE_JAVASCRIPT_COMPAT;
		//fall through
	case TYPE_REGEX_PCRE:
		if (f & PFLAG_ICASE){
			flags_new |= PCRE_CASELESS;
		}
		break;
	}

	return flags_new;
}

int pat_init(const char* pattern, struct pattern* in_out, unsigned flags){
	int res;
	const char* err;
	int ret = 0;
	int flags_new = flags_convert(in_out->p_type, flags);

	switch (in_out->p_type){
	case TYPE_FNMATCH:
	case TYPE_FNMATCH_ESCAPE:
	case TYPE_FNMATCH_LITERAL:
		in_out->p.fnmatch = pattern;
		break;

	case TYPE_REGEX_POSIX:
	case TYPE_REGEX_POSIX_EX:
		res = regcomp(in_out->p.regex, pattern, flags_new);
		if (res != 0){
			char errbuf[256];
			regerror(res, in_out->p.regex, errbuf, sizeof(errbuf));
			eprintf_mt("ffind: Failed to create posix regex (%s)\n", errbuf);
			ret = -1;
			break;
		}
		break;

	case TYPE_REGEX_PCRE:
		in_out->p.pcre = pcre_compile(pattern, flags_new, &err, &res, NULL);
		if (!in_out->p.pcre){
			eprintf_mt("ffind: Failed to compile pcre regex at character %d (%s)\n", res, err);
			ret = -1;
			break;
		}
		break;

	case TYPE_REGEX_JAVASCRIPT:
		in_out->p.javascript = pcre_compile(pattern, flags_new, &err, &res, NULL);

		if (!in_out->p.javascript){
			eprintf_mt("ffind: Failed to compile javascript regex at character %d (%s)\n", res, err);
			ret = -1;
			break;
		}
		break;

	}

	return ret;
}

void pat_free(struct pattern* pat){
	if (!pat){
		return;
	}

	switch (pat->p_type){
	case TYPE_FNMATCH:
	case TYPE_FNMATCH_ESCAPE:
	case TYPE_FNMATCH_LITERAL:
		return;
	case TYPE_REGEX_POSIX:
	case TYPE_REGEX_POSIX_EX:
		if (pat->p.regex){
			regfree(pat->p.regex);
		}
		return;
	case TYPE_REGEX_PCRE:
		if (pat->p.pcre){
			pcre_free(pat->p.pcre);
		}
		return;
	case TYPE_REGEX_JAVASCRIPT:
		if (pat->p.javascript){
			pcre_free(pat->p.javascript);
		}
		return;
	}
}
