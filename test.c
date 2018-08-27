/*
 * Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "ffind_match.h"
#include <stdio.h>
#include <assert.h>

void test_match_wildcard(void){
	const char* haystack = "/home/equifax/passwords.txt";
	assert(match_wildcard(haystack, haystack) == 1);
	assert(match_wildcard(haystack, "/home/nomatchhere") == 0);
	assert(match_wildcard(haystack, "/home/equifax/passwords") == 0);
	assert(match_wildcard(haystack, "/home/equifax/passwords.txtt") == 0);

	assert(match_wildcard(haystack, "*") == 1);
	assert(match_wildcard(haystack, "*.txt") == 1);
	assert(match_wildcard(haystack, "/home/*/passwords.txt") == 1);
	assert(match_wildcard(haystack, "/home/*/*.txt") == 1);
	assert(match_wildcard(haystack, "/home/*/*.txt*") == 1);
	assert(match_wildcard(haystack, "*pass*words.t*") == 1);
	assert(match_wildcard(haystack, "*pass*words.t") == 0);
	assert(match_wildcard(haystack, "passwords.txt*") == 0);
	assert(match_wildcard(haystack, "*paword*.txt*s*") == 0);
}

int main(void){
	test_match_wildcard();
}
