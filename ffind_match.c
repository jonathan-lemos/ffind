/*
 * Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <stdlib.h>
#include <string.h>

/* Returns true if the needle is within the haystack, false if not.
 *
 * The needle can include the '*' character, which matches 0 or more of any character.
 * A literal '*' can be inputted with "\*"
 */
int match_wildcard(const char* haystack, const char* needle){
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
