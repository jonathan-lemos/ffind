/*
 * Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include <stdlib.h>
#include <string.h>

int match_wildcard(const char* haystack, const char* needle){
	char* needle_buf;
	char* token;

	needle_buf = malloc(strlen(needle) + 1);
	if (!needle_buf){
		return -1;
	}
	strcpy(needle_buf, needle);
	token = strtok(needle_buf, "*");
	if (!token){
		free(needle_buf);
		return 1;
	}

	if (needle[0] != '*' && strstr(haystack, token) != haystack){
		free(needle_buf);
		return 0;
	}

	while (token){
		char* tmp = strstr(haystack, token);
		if (!tmp){
			free(needle_buf);
			return 0;
		}
		haystack = tmp + strlen(token);
		token = strtok(NULL, "*");
	}

	if (strlen(haystack) != 0 && needle[strlen(needle) - 1] != '*'){
		free(needle_buf);
		return 0;
	}

	free(needle_buf);
	return 1;
}
