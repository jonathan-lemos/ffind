/** @file log.h
 * @brief Thread-safe printing functions.
 * @copyright Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef __LOG_H
#define __LOG_H

#include "attribute.h"
#include <stdarg.h>

#define log_enomem()      eprintf_mt("ffind: failed to allocate requested memory\n")
#define log_eopendir(dir) eprintf_mt("ffind: failed to open %s (%s)\n", dir, strerror(errno))
#define log_ethread()     eprintf_mt("ffind: failed to start thread (%s)\n", strerror(errno))
#define log_ejoin()       eprintf_mt("ffind: failed to join thread (%s)\n", strerror(errno))

/**
 * @brief Thread-safe printf.<br>
 * Only one thread will be allowed to print through this function at a time.<br>
 * This does not protect against calls to printf()/fprintf(), but it does protect against calls to printf_mt()/eprintf_mt().
 *
 * @param format The printf format string.
 *
 * @param ... The printf arguments, if any.
 *
 * @return The number of characters successfully written.
 */
int printf_mt(const char* format, ...) FF_HOT FF_PRINTF_LIKE(1);

/**
 * @brief Thread-safe fprintf(stderr).<br>
 * Only one thread will be allowed to print through this function at a time.<br>
 * This does not protect against calls to printf()/fprintf(), but it does protect against calls to printf_mt()/eprintf_mt().
 *
 * @param format The printf format string.
 *
 * @param ... The printf arguments, if any.
 *
 * @return The number of characters successfully written.
 */
int eprintf_mt(const char* format, ...) FF_COLD FF_PRINTF_LIKE(1);

#endif
