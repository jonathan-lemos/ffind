/*
 * Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "flags.h"
#include "log.h"
#include "match.h"
#include <stdlib.h>
#include <string.h>

static char* str_dup(const char* s){
	char* ret = malloc(strlen(s) + 1);
	if (!ret){
		return NULL;
	}
	strcpy(ret, s);
	return ret;
}

static int add_string(const char* s, char*** array_ptr, size_t* array_len){
	void* tmp;
	char* tmp_str;

	tmp_str = str_dup(s);
	if (!tmp_str){
		log_enomem();
		return -1;
	}

	(*array_len)++;
	tmp = realloc(*array_ptr, *array_len);
	if (!tmp){
		log_enomem();
		(*array_len)--;
		free(tmp_str);
		return -1;
	}
	*array_ptr = tmp;
	*array_ptr[*array_len - 1] = tmp_str;
	return 0;
}

static void flags_init(struct ffind_flags* in_out){
	in_out->matcher = match_wildcard;
	in_out->type = '\0';
	in_out->follow_symlink = 0;
	in_out->print0 = 0;
}

static void display_help(const char* prog_name){
	printf_mt("Usage: %s [-H] [-L] [-P] [directory...] [pattern]\n", prog_name);
}

static void display_version(void){
	printf_mt("ffind 0.1\n"
		   "Copyright (c) 2018 Jonathan Lemos\n"
		   "This program is licensed under the MIT License\n");
}

int parse_options(int argc, char** argv, struct ffind_flags* flags_out, char*** dir_out, size_t* dir_out_len, char** pattern_out){
	int ret = 0;
	char** dir = NULL;
	char* pattern = NULL;
	size_t dir_len = 0;

	flags_init(flags_out);

	for (int i = 1; i < argc;  ++i){
		if (strcmp(argv[i], "--help")){
			display_help(argv[0]);
			ret = 1;
			goto cleanup;
		}

		else if (strcmp(argv[i], "-name")){
			i++;
			if (pattern){
				eprintf_mt("ffind: -name and -regex options cannot both be specified.\n");
				ret = -1;
				goto cleanup;
			}
			pattern = malloc(strlen(argv[i]) + 1);
			if (pattern){
				log_enomem();
				ret = -1;
				goto cleanup;
			}
			strcpy(pattern, argv[i]);
			flags_out->matcher = match_wildcard;
		}

		else if (strcmp(argv[i], "-print0")){
			flags_out->print0 = 1;
		}

		else if (strcmp(argv[i], "-regex")){
			i++;
			if (pattern){
				eprintf_mt("ffind: -name and -regex options cannot both be specified/\n");
				ret = -1;
				goto cleanup;
			}
			pattern = malloc(strlen(argv[i]) + 1);
			if (pattern){
				log_enomem();
				ret = -1;
				goto cleanup;
			}
			strcpy(pattern, argv[i]);
			flags_out->matcher = match_regex;
		}

		else if (strcmp(argv[i], "-type")){
			i++;
			if (strlen(argv[i]) != 1){
				eprintf_mt("ffind: -type argument should only contain 1 letter\n");
				ret = -1;
				goto cleanup;
			}
			switch (argv[i][0]){
			case 'd':
			case 'f':
				break;
			default:
				eprintf_mt("ffind: -type argument %c is not supported\n", argv[i][0]);
			}
			flags_out->type = argv[i][0];
		}

		else if (strcmp(argv[i], "--version")){
			display_version();
			ret = 1;
			goto cleanup;
		}

		else if (argv[i][0] == '-'){
			for (size_t j = 1; j < strlen(argv[i]); ++j){
				switch(argv[i][j]){
				case 'P':
					flags_out->follow_symlink = 0;
					break;
				case 'L':
				case 'H':
					flags_out->follow_symlink = 1;
					break;
				default:
					eprintf_mt("ffind: -%c argument is not supported\n", argv[i][j]);
					ret = -1;
					goto cleanup;
				}
			}
		}
	}

cleanup:
	if (ret != 0){
		for (size_t i = 0; i < dir_len; ++i){
			free(dir_out[i]);
		}
		free(dir_out);
		free(pattern_out);
	}
	else{
		*dir_out = dir;
		*pattern_out = pattern;
		*dir_out_len = dir_len;
	}
	return ret;
}
