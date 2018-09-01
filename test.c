/*
 * Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "match.h"
#include <stdio.h>
#include <assert.h>

void test_match_wildcard(void){
	assert(match_wildcard("/home/equifax/passwords.txt", "/home/equifax/passwords.txt") == 1);
	assert(match_wildcard("/home/equifax/passwords.txt", "/home/nomatchhere") == 0);
	assert(match_wildcard("/home/equifax/passwords.txt", "/home/equifax/passwords") == 0);
	assert(match_wildcard("/home/equifax/passwords.txt", "/home/equifax/passwords.txtt") == 0);

	assert(match_wildcard("/home/equifax/passwords.txt", "*") == 1);
	assert(match_wildcard("/home/equifax/passwords.txt", "*.txt") == 1);
	assert(match_wildcard("/home/equifax/passwords.txt", "/home/*/passwords.txt") == 1);
	assert(match_wildcard("/home/equifax/passwords.txt", "/home/*/*.txt") == 1);
	assert(match_wildcard("/home/equifax/passwords.txt", "/home/*/*.txt*") == 1);
	assert(match_wildcard("/home/equifax/passwords.txt", "*pass*words.t*") == 1);
	assert(match_wildcard("/home/equifax/passwords.txt", "*pass*words.t") == 0);
	assert(match_wildcard("/home/equifax/passwords.txt", "passwords.txt*") == 0);
	assert(match_wildcard("/home/equifax/passwords.txt", "*paword*.txt*s*") == 0);

	assert(match_wildcard("/home/**.txt*", "/home/\\*\\*.txt\\*") == 1);
	assert(match_wildcard("/home/**.txt*", "/home/*.tx\\*") == 0);
	assert(match_wildcard("/home/**.txt*", "/home/*") == 1);
}

int main(void){
	test_match_wildcard();
}
