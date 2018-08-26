#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dirent.h>
#include <stdint.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdarg.h>

#ifndef __GNUC__
#define __attribute__(x)
#endif

#define N_THREADS (8)

#define log_enomem()      ts_eprintf("ffind: failed to allocate requested memory\n")
#define log_eopendir(dir) ts_eprintf("ffind: failed to open %s (%s)\n", dir, strerror(errno))
#define log_ethread()     ts_eprintf("ffind: failed to start thread (%s)\n", strerror(errno))
#define log_ejoin()       ts_eprintf("ffind: failed to join thread (%s)\n", strerror(errno))

pthread_mutex_t mutex_dir_stack = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex_stdio = PTHREAD_MUTEX_INITIALIZER;

char** dir_stack = NULL;
size_t dir_stack_len = 0;

struct ffind_param{
	const char* find_me;
	uint16_t flags;
};

/* The path matching logic.
 * Currently it only checks if needle is a substring of haystack.
 * TODO: implement asterisk/regex */
int path_matches(const char* haystack, const char* needle){
	return strstr(haystack, needle) != NULL;
}

/* Pushes a directory on to the stack.
 * This string must be allocated with malloc().
 * This function is thread-safe. */
int dir_stack_push(char* entry){
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
char* dir_stack_pop(void){
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
void dir_stack_free(void){
	pthread_mutex_lock(&mutex_dir_stack);

	for (size_t i = 0; i < dir_stack_len; ++i){
		free(dir_stack[i]);
	}
	free(dir_stack);
	dir_stack = NULL;
	dir_stack_len = 0;

	pthread_mutex_unlock(&mutex_dir_stack);
}

/* Thread-safe printf.
 * Waits for the stdio mutex before printing.
 */
__attribute__((format(printf, 1, 2)))
int ts_printf(const char* format, ...){
	int res;
	va_list ap;

	pthread_mutex_lock(&mutex_stdio);
	va_start(ap, format);

	res = vprintf(format, ap);

	va_end(ap);
	pthread_mutex_unlock(&mutex_stdio);
	return res;
}

/* Thread-safe fprintf(stderr).
 * Waits for the stdio mutex before printing.
 */
__attribute__((format(printf, 1, 2)))
int ts_eprintf(const char* format, ...){
	int res;
	va_list ap;

	va_start(ap, format);
	res = vfprintf(stderr, format, ap);
	va_end(ap);
	return res;
}

/* Creates a path out of a directory and dirent.d_name
 * This string must be free()'d after use.
 */
char* make_path(const char* dir, const char* d_name){
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

/* The main finding function.
 * Finds all files in a directory that match ffp->find_me
 */
int ffind_backend(const char* base_dir, const char* find_me, uint16_t flags){
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

		stat(path, &st);
		if (S_ISDIR(st.st_mode)){
			ffind_backend(path, find_me, flags);
		}
		else if (path_matches(path, find_me)){
			/* TODO: implement -print0 */
			ts_printf("%s\n", path);
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
		ffind_backend(current_dir, ffp->find_me, ffp->flags);
		free(current_dir);
	}

	return NULL;
}

/* Because only 1 thread can run in a directory, the initial search is always single threaded.
 * TODO: Make the initial search multi-threaded. */
int ffind_init_stack(const char* base_dir, const char* find_me, uint16_t flags){
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

		stat(path, &st);
		if (S_ISDIR(st.st_mode)){
			if (dir_stack_push(path) != 0){
				log_enomem();
				dir_stack_free();
				free(path);
				closedir(dp);
				return -1;
			}
		}
		else{
			if (path_matches(find_me, path)){
				/* TODO: implement -print0 */
				ts_printf("%s\n", path);
			}
			free(path);
		}
	}

	closedir(dp);
	return 0;
}

int main(int argc, char** argv){
	pthread_t* threads;
	size_t threads_len = N_THREADS;
	struct ffind_param ffp;
	uint16_t flags = 0;

	if (argc < 3){
		printf("Usage: %s [directory] [pattern]\n", argv[0]);
		return 1;
	}

	if (ffind_init_stack(argv[1], argv[2], flags) != 0){
		fprintf(stderr, "ffind: Failed to initialize\n");
		return 1;
	}

	threads = malloc(threads_len * sizeof(*threads));
	if (!threads){
		log_enomem();
		return 1;
	}

	ffp.find_me = argv[2];
	ffp.flags = flags;

	for (size_t i = 0; i < threads_len; ++i){
		if (pthread_create(&(threads[i]), NULL, ffind_worker_thread, &ffp) != 0){
			log_ethread();
			return -1;
		}
	}

	for (size_t i = 0; i < threads_len; ++i){
		if (pthread_join(threads[i], NULL) != 0){
			log_ejoin();
		}
	}

	return 0;
}
