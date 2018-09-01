/*
 * Copyright (c) 2018 Jonathan Lemos
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "ffind.h"
#include "flags.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

int main(int argc, char** argv){
	pthread_t* threads;
	struct parsed_data pd;
	int res;

	res = parse_options(argc, argv, &pd);
	if (res > 0){
		return 0;
	}
	else if (res < 0){
		return 1;
	}

	for (size_t i = 0; i < pd.directories_len; ++i){
		if (ffind_create_threads(pd.directories[i], &pd, &threads) != 0){
			free_options(&pd);
			return 1;
		}
		if (ffind_join_threads(threads, pd.n_threads) != 0){
			free_options(&pd);
			return 1;
		}
	}
	free_options(&pd);
	return 0;
}
