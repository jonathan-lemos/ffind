/** @file ffind.c
 * @brief The finding backend.
 * @copyright Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "ffind.h"
#include "match.h"
#include "options.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdarg.h>

static pthread_mutex_t mutex_dir_stack = PTHREAD_MUTEX_INITIALIZER;

char** dir_stack = NULL;
size_t dir_stack_len = 0;

struct ffind_param{
	const struct pattern* p;
	const struct ffind_flags* flags;
	int maxdepth;
};

/* Pushes a directory on to the stack.
 * This string must be allocated with malloc().
 * This function is thread-safe. */
static int dir_stack_push(char* entry){
	void* tmp;
	pthread_mutex_lock(&mutex_dir_stack);

	dir_stack_len++;

	/* can't use dir_stack = realloc(dir_stack, ...).
	 * if realloc fails in the above case, the pointer (which we lost though assignment) is still valid, causing a mem leak. */
	tmp = realloc(dir_stack, dir_stack_len * sizeof(*dir_stack));
	if (!tmp){
		pthread_mutex_unlock(&mutex_dir_stack);
		return -1;
	}
	dir_stack = tmp;

	dir_stack[dir_stack_len - 1] = entry;

	pthread_mutex_unlock(&mutex_dir_stack);
	return 0;
}

/* Pops the top directory off the stack.
 * This string must be free()'d after use.
 * This function is thread-safe. */
static char* dir_stack_pop(void){
	char* ret;
	void* tmp;
	pthread_mutex_lock(&mutex_dir_stack);

	if (dir_stack_len == 0){
		pthread_mutex_unlock(&mutex_dir_stack);
		return NULL;
	}

	ret = dir_stack[dir_stack_len - 1];
	dir_stack_len--;

	tmp = realloc(dir_stack, dir_stack_len * sizeof(*dir_stack));
	/* if dir_stack_len == 0, tmp is NULL */
	if (tmp || dir_stack_len == 0){
		dir_stack = tmp;
	}

	/* don't care if realloc fails, since all that happens is a slight waste of memory */
	pthread_mutex_unlock(&mutex_dir_stack);
	return ret;
}

/* Frees every entry in a directory stack
 * This function is thread-safe. */
FF_UNUSED static void dir_stack_free(void){
	pthread_mutex_lock(&mutex_dir_stack);

	for (size_t i = 0; i < dir_stack_len; ++i){
		free(dir_stack[i]);
	}
	free(dir_stack);
	dir_stack = NULL;
	dir_stack_len = 0;

	pthread_mutex_unlock(&mutex_dir_stack);
}

/* Creates a path out of a directory and dirent.d_name
 * This string must be free()'d after use.
 */
FF_HOT FF_INLINE static char* make_path(const char* dir, const char* d_name){
	char* path = malloc(strlen(dir) + strlen(d_name) + 2);
	if (!path){
		return NULL;
	}

	strcpy(path, dir);
	if (path[strlen(path) - 1] != '/'){
		strcat(path, "/");
	}
	strcat(path, d_name);
	return path;
}

FF_INLINE static void print_match(const char* path, mode_t st_mode, const struct pattern* pat, char type, unsigned print0){
	switch (type){
	case 'f':
		if (!S_ISREG(st_mode)){
			return;
		}
		break;
	case 'd':
		if (!S_ISDIR(st_mode)){
			return;
		}
	}

	if (match(path, pat) == 1){
		switch (print0){
			case 0:
				printf("%s\n", path);
				break;
			case 1:
				fwrite(path, 1, strlen(path) + 1, stdout);
		}
	}
}

/* The main finding function.
 * Finds all files in a directory that match ffp->find_me
 */
FF_HOT int ffind_backend(const char* base_dir, const struct pattern* pattern, const struct ffind_flags* ffl, int max_depth){
	DIR* dp;
	struct dirent* dnt;

	if (max_depth == 0){
		return 0;
	}

	dp = opendir(base_dir);
	if (!dp){
		log_eopendir(base_dir);
		return -1;
	}

	while ((dnt = readdir(dp)) != NULL){
		char* path;
		struct stat st;

		/* "." and ".." are symlinks to the current directory/parent directory.
		 * we do not want to search through these, as it would cause an infinite loop */
		if (!strcmp(dnt->d_name, ".") || !strcmp(dnt->d_name, "..")){
			continue;
		}

		path = make_path(base_dir, dnt->d_name);
		if (!path){
			log_enomem();
			closedir(dp);
			return -1;
		}

		switch (ffl->follow_symlink){
		case 0:
			lstat(path, &st);
			break;
		default:
			stat(path, &st);
			break;
		}
		print_match(path, st.st_mode, pattern, ffl->type, ffl->print0);
		if (S_ISDIR(st.st_mode)){
			ffind_backend(path, pattern, ffl, max_depth - 1);
		}
		free(path);
	}

	closedir(dp);
	return 0;
}

void* ffind_worker_thread(void* param){
	const struct ffind_param* ffp = param;
	char* current_dir;

	while ((current_dir = dir_stack_pop()) != NULL){
		ffind_backend(current_dir, ffp->p, ffp->flags, ffp->maxdepth);
		free(current_dir);
	}

	return NULL;
}

/* Because only 1 thread can run in a directory, the initial search is always single threaded.
 * TODO: Make the initial search multi-threaded. */
int ffind_init_stack(const char* base_dir, const struct pattern* pattern, const struct ffind_flags* ffl){
	DIR* dp;
	struct dirent* dnt;

	dp = opendir(base_dir);
	if (!dp){
		log_eopendir(base_dir);
		return -1;
	}

	while ((dnt = readdir(dp)) != NULL){
		char* path;
		struct stat st;

		/* "." and ".." are symlinks to the current directory/parent directory.
		 * we do not want to search through these, as it would cause an infinite loop */
		if (!strcmp(dnt->d_name, ".") || !strcmp(dnt->d_name, "..")){
			continue;
		}

		path = make_path(base_dir, dnt->d_name);
		if (!path){
			log_enomem();
			closedir(dp);
			return -1;
		}

		switch (ffl->follow_symlink){
		case 0:
			lstat(path, &st);
			break;
		default:
			stat(path, &st);
			break;
		}
		print_match(path, st.st_mode, pattern, ffl->type, ffl->print0);
		if (S_ISDIR(st.st_mode)){
			dir_stack_push(path);
		}
		else{
			free(path);
		}
	}

	closedir(dp);
	return 0;
}

int ffind_create_threads(const char* base_dir, const struct parsed_data* pd, pthread_t** out){
	static struct ffind_param ffp;
	pthread_t* threads = NULL;
	int ret = 0;

	if (pd->n_threads == 0){
		eprintf_mt("ffind: Cannot start with 0 threads.\n");
		ret = -1;
		goto cleanup;
	}

	if (ffind_init_stack(base_dir, &(pd->pat), &(pd->flags)) != 0){
		ret = -1;
		goto cleanup;
	}

	threads = malloc(pd->n_threads * sizeof(*threads));
	if (!threads){
		log_enomem();
		ret = -1;
		goto cleanup;
	}

	ffp.p = &(pd->pat);
	ffp.flags = &(pd->flags);
	ffp.maxdepth = pd->maxdepth;

	for (size_t i = 0; i < pd->n_threads; ++i){
		if (pthread_create(&(threads[i]), NULL, ffind_worker_thread, &ffp) != 0){
			log_ethread();
			free(threads);
			ret = -1;
			goto cleanup;
		}
	}

cleanup:
	if (ret != 0){
		free(threads);
		*out = NULL;
	}
	else{
		*out = threads;
	}
	return ret;
}

int ffind_join_threads(pthread_t* threads, size_t threads_len){
	int ret = 0;
	for (size_t i = 0; i < threads_len; ++i){
		if (pthread_join(threads[i], NULL) != 0){
			log_ejoin();
			ret = -1;
		}
	}
	free(threads);
	return ret;
}
