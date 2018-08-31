/*
 * Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "log.h"
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

pthread_mutex_t mutex_stdio = PTHREAD_MUTEX_INITIALIZER;

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

	pthread_mutex_lock(&mutex_stdio);
	va_start(ap, format);

	res = vfprintf(stderr, format, ap);

	va_end(ap);
	pthread_mutex_unlock(&mutex_stdio);
	return res;
}
