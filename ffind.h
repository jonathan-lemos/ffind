/** @file ffind.h
 * @brief The finding backend.
 * Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef __FFIND_H
#define __FFIND_H

#include "options.h"
#include "match.h"
#include <pthread.h>

/**
 * @brief Creates the ffind threads.
 *
 * @param base_dir The directory to start iterating through.
 *
 * @param pd A pointer to a flags structure filled by parse_options().<br>
 * This cannot be NULL.
 * @see parse_options()
 *
 * @param out A pointer to a pointer of pthreads that will be filled by this function.<br>
 * This must be freed with ffind_join_threads()
 * @see ffind_join_threads()
 *
 *
 * @return 0 on success, negative on failure.
 * On failure, out is set to NULL and does not have to be freed.
 */
int ffind_create_threads(const char* base_dir, const struct parsed_data* pd, pthread_t** out);

/**
 * @brief Waits for ffind threads to finish and releases all memory associated with them.<br>
 * These threads should be created by ffind_create_threads()
 * @see ffind_create_threads()
 *
 * @param threads The ffind threads to wait for.
 *
 * @param threads_len The number of threads created.<br>
 * This is equal to the parsed_data structure's "n_threads" member at the time of the threads' creation.
 *
 * @return 0 on success, negative on failure.
 * On failure, the memory is still released and the threads should not be reused.
 */
int ffind_join_threads(pthread_t* threads, size_t threads_len);

#endif
