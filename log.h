/*
 * Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef __LOG_H
#define __LOG_H

#ifndef __GNUC__
#define __attribute__(x)
#endif

#include <stdarg.h>

#define log_enomem()      eprintf_mt("ffind: failed to allocate requested memory\n")
#define log_eopendir(dir) eprintf_mt("ffind: failed to open %s (%s)\n", dir, strerror(errno))
#define log_ethread()     eprintf_mt("ffind: failed to start thread (%s)\n", strerror(errno))
#define log_ejoin()       eprintf_mt("ffind: failed to join thread (%s)\n", strerror(errno))

/* Thread-safe printf. */
int printf_mt(const char* format, ...) __attribute__((format(printf, 1, 2)));
/* Thread-safe fprintf(stderr). */
int eprintf_mt(const char* format, ...) __attribute__((format(printf, 1, 2)));

#endif
