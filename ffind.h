/*
 * Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef __FFIND_H
#define __FFIND_H

#include "flags.h"
#include "match.h"
#include <pthread.h>

int ffind_create_threads(const char* base_dir, const struct parsed_data* pd, pthread_t** out);
int ffind_join_threads(pthread_t* threads, size_t threads_len);

#endif
