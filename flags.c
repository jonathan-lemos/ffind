/*
 * Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "flags.h"
#include "log.h"
#include "match.h"
#include <stdio.h>
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
	tmp = realloc(*array_ptr, *array_len * sizeof(**array_ptr));
	if (!tmp){
		log_enomem();
		(*array_len)--;
		free(tmp_str);
		return -1;
	}
	*array_ptr = tmp;
	(*array_ptr)[*array_len - 1] = tmp_str;
	return 0;
}

static void pd_init(struct parsed_data* pd){
	pd->flags.type = '\0';
	pd->flags.follow_symlink = 0;
	pd->flags.print0 = 0;
	pd->directories = NULL;
	pd->directories_len = 0;
	pd->pat.p_type = TYPE_REGEX_POSIX;
	pd->pat.p.fnmatch = NULL;
	pd->maxdepth = -1;
	pd->n_threads = 4;
}

static void display_help(const char* prog_name){
	printf_mt("Usage: %s [options] [directory...] [pattern]\n", prog_name);
	printf_mt("Options\n");
	printf_mt("\t-e: Allow escape characters with -name argument\n");
	printf_mt("\t-H: Follow symbolic links.\n");
	printf_mt("\t-I: Ignore case when searching.\n");
	printf_mt("\t-l: Treat the -name argument literally and match if it is a substring.\n");
	printf_mt("\t-jNUMBER: Use a specified number of threads.\n");
	printf_mt("\t-L: Follow symbolic links (same as -H).\n");
	printf_mt("\t-P: Do not follow symbolic links.\n");
	printf_mt("\t-maxdepth NUMBER: Set the maximum recursion depth\n");
	printf_mt("\t-name PATTERN: Find files matching this pattern.\n");
	printf_mt("\t-regex PATTERN: Find files matching this regular expression.\n");
	printf_mt("\t-regextype TYPE: Use a different regex dialect. Use \"-regextype help\" to see available dialects.\n");
	printf_mt("\t-type df:\n"
			"\t\t-type d: Match directories only.\n"
			"\t\t-type f: Match files only.\n");
}

static void display_help_regex(void){
	printf_mt("Regex types: 'default', 'egrep', 'grep', 'javascript', 'posix-basic', 'posix-extended', 'pcre', 'python'\n");
}

static void display_version(void){
	printf_mt("ffind 0.1 beta\n");
	printf_mt("Copyright (c) 2018 Jonathan Lemos\n");
	printf_mt("This program is licensed under the MIT License\n");
}

int parse_options(int argc, char** argv, struct parsed_data* in_out){
	char* pat_text = NULL;
	int p_flags = 0;
	int ret = 0;

	pd_init(in_out);

	for (int i = 1; i < argc; ++i){
		if (!strcmp(argv[i], "--help")){
			display_help(argv[0]);
			ret = 1;
			goto cleanup;
		}

		else if (!strcmp(argv[i], "-maxdepth")){
			char* tmp;
			i++;
			in_out->maxdepth = strtol(argv[i], &tmp, 10);
			if (tmp == argv[i]){
				eprintf_mt("ffind: -maxdepth must be a number.");
				ret = -1;
				goto cleanup;
			}
		}

		else if (!strcmp(argv[i], "-name")){
			i++;
			pat_text = argv[i];
			switch (in_out->pat.p_type){
				case TYPE_FNMATCH:
				case TYPE_FNMATCH_ESCAPE:
				case TYPE_FNMATCH_LITERAL:
					break;
				default:
					in_out->pat.p_type = TYPE_FNMATCH;
			}
		}

		else if (!strcmp(argv[i], "-print0")){
			in_out->flags.print0 = 1;
		}

		else if (!strcmp(argv[i], "-regex")){
			i++;
			pat_text = argv[i];
			switch (in_out->pat.p_type){
			case TYPE_FNMATCH:
			case TYPE_FNMATCH_ESCAPE:
			case TYPE_FNMATCH_LITERAL:
				in_out->pat.p_type = TYPE_REGEX_POSIX;
				break;
			default:
				;
			}
		}

		else if (!strcmp(argv[i], "-regextype")){
			i++;

			if (!strcmp(argv[i], "help")){
				display_help_regex();
				ret = 1;
				goto cleanup;
			}

			else if (!strcmp(argv[i], "default") ||
					!strcmp(argv[i], "posix-basic") ||
					!strcmp(argv[i], "grep")){
				in_out->pat.p_type = TYPE_REGEX_POSIX;
			}

			else if (!strcmp(argv[i], "posix-extended") ||
					!strcmp(argv[i], "egrep")){
				in_out->pat.p_type = TYPE_REGEX_POSIX_EX;
			}

			else if (!strcmp(argv[i], "pcre") ||
					!strcmp(argv[i], "python")){
				in_out->pat.p_type = TYPE_REGEX_PCRE;
			}

			else if (!strcmp(argv[i], "javascript")){
				in_out->pat.p_type = TYPE_REGEX_JAVASCRIPT;
			}

			else{
				eprintf_mt("Invalid regex type: %s\n", argv[i]);
				display_help_regex();
				ret = 1;
				goto cleanup;
			}
		}

		else if (!strcmp(argv[i], "-type")){
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
			in_out->flags.type = argv[i][0];
		}

		else if (!strcmp(argv[i], "--version")){
			display_version();
			ret = 1;
			goto cleanup;
		}

		else if (argv[i][0] == '-'){
			char buf[16] = {'\0'};
			size_t buf_ptr = 0;
			for (size_t j = 1; j < strlen(argv[i]); ++j){
				switch(argv[i][j]){
				case 'e':
					in_out->pat.p_type = TYPE_FNMATCH_ESCAPE;
					break;
				case 'I':
					p_flags |= PFLAG_ICASE;
					break;
				case 'j':
					j++;
					for (; argv[i][j] >= '0' && argv[i][j] <= '9' && buf_ptr <= sizeof(buf) - 1; ++j, ++buf_ptr){
						buf[buf_ptr] = argv[i][j];
					}
					if (buf_ptr == sizeof(buf)){
						eprintf_mt("ffind: Too many threads specified. Lower the number passed to the -j argument.\n");
					}
					if (sscanf(buf, "%zu", &(in_out->n_threads)) != 1){
						eprintf_mt("ffind: Character(s) directly after -j must make up a number.\n");
						ret = -1;
						goto cleanup;
					}
					break;
				case 'l':
					in_out->pat.p_type = TYPE_FNMATCH_LITERAL;
					break;
				case 'L':
					in_out->flags.follow_symlink = 1;
					break;
				case 'H':
				case 'P':
					in_out->flags.follow_symlink = 0;
					break;
				default:
					eprintf_mt("ffind: -%c argument is not supported\n", argv[i][j]);
					ret = -1;
					goto cleanup;
				}
			}
		}

		else{
			if (add_string(argv[i], &(in_out->directories), &(in_out->directories_len)) != 0){
				ret = -1;
				goto cleanup;
			}
		}
	}

	if (!pat_text){
		pat_text = "*";
		in_out->pat.p_type = TYPE_FNMATCH;
	}

	if (!(in_out->directories)){
		if (add_string(".", &(in_out->directories), &(in_out->directories_len)) != 0){
			ret = -1;
			goto cleanup;
		}
	}

	if (pat_init(pat_text, &(in_out->pat), p_flags) != 0){
		ret = -1;
		goto cleanup;
	}

cleanup:
	if (ret != 0){
		free_options(in_out);
	}
	return ret;
}

void free_options(struct parsed_data* pd){
	if (!pd){
		return;
	}
	for (size_t i = 0; i < pd->directories_len; ++i){
		free(pd->directories[i]);
	}
	free(pd->directories);
	pat_free(&(pd->pat));
}
