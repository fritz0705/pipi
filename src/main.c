/* pipi: Some pi generation program working with gmplib
 * Copyright (c) 2012 Fritz Grimpen
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <gmp.h>
#include <pthread.h>

#include "bbp-pi.h"
#include "bellard.h"

enum pi_algo
{
	ALGO_BBP,
	ALGO_3,
	ALGO_BELLARD
};

struct thread_info
{
	pthread_t thread_id;
	enum pi_algo algorithm;
	
	mpz_t start;
	mpz_t rounds;
	mpq_t result;
};

static struct {
	int threads;
	int base;
	mpz_t digits;
	mpz_t rounds;
	enum pi_algo algorithm;
} state = {
	.threads = 1,
	.base = 10,
	.algorithm = ALGO_BBP
};

static void parse_argv(int argc, char **argv)
{
	mpz_init(state.digits);
	mpz_init(state.rounds);
	mpz_set_ui(state.digits, 64);
	mpz_set_ui(state.rounds, 64);
	state.threads = 1;

	int pstate = 0;
	for (int i = 1; i < argc; ++i)
	{
		switch (pstate)
		{
			case 0:
				if (!strcmp(argv[i], "-threads"))
					pstate = 1;
				else if (!strcmp(argv[i], "-algo"))
					pstate = 2;
				else if (!strcmp(argv[i], "-base"))
					pstate = 3;
				else if (!strcmp(argv[i], "-rounds"))
					pstate = 4;
				else if (argv[i][0] == '-')
					fprintf(stderr, "Invalid argument at position %d: %s\n", i, argv[i]);
				else
				{
					mpz_set_str(state.digits, argv[i], 10);
					mpz_set(state.rounds, state.digits);
					mpz_sub_ui(state.rounds, state.rounds, 1);
				}
				break;
			case 1:
				state.threads = atoi(argv[i]);
				pstate = 0;
				break;
			case 2:
				if (!strcmp(argv[i], "bbp"))
					state.algorithm = ALGO_BBP;
				else if (!strcmp(argv[i], "3"))
					state.algorithm = ALGO_3;
				else if (!strcmp(argv[i], "bellard"))
					state.algorithm = ALGO_BELLARD;
				pstate = 0;
				break;
			case 3:
				state.base = atoi(argv[i]);
				pstate = 0;
				break;
			case 4:
				mpz_set_str(state.rounds, argv[i], 10);
		}
	}
}

static void print_mpq(mpq_t num, mpz_t digits, unsigned int base)
{
	mpz_t numer;
	mpz_t denum;
	mpz_init(numer);
	mpz_init(denum);

	mpq_get_num(numer, num);
	mpq_get_den(denum, num);

	mpz_t int_base;
	mpz_init_set_ui(int_base, base);

	mpz_t int_0;
	mpz_init_set_ui(int_0, 0);

	mpz_t mult;
	mpz_init(mult);

	unsigned int digits_int = mpz_get_ui(digits);
	mpz_pow_ui(mult, int_base, digits_int);

	mpz_t new_numer;
	mpz_init(new_numer);
	mpz_mul(new_numer, numer, mult);

	mpz_t result;
	mpz_init(result);

	mpz_div(result, new_numer, denum);

	mpz_out_str(stdout, base, result);
	printf("\n");

	mpz_clear(int_base);
	mpz_clear(int_0);
	mpz_clear(mult);
	mpz_clear(new_numer);
	mpz_clear(result);
	mpz_clear(numer);
	mpz_clear(denum);
}

static void *thread_main(struct thread_info *info)
{
	switch (info->algorithm)
	{
		case ALGO_BBP:
			bbp_pi(info->start, info->rounds, info->result);
			break;
		case ALGO_3:
			break;
		case ALGO_BELLARD:
			bellard_pi(info->start, info->rounds, info->result);
			break;
	}
	return NULL;
}

int main(int argc, char **argv)
{
	parse_argv(argc, argv);

	struct thread_info *tinfos = calloc(state.threads, sizeof(struct thread_info));
	if (tinfos == NULL)
	{
		perror("Could not allocate thread infos");
		exit(1);
	}

	/* Some threading attributes */
	pthread_attr_t pattr;
	pthread_attr_init(&pattr);

	/* Rounds per thread */
	mpz_t roundspt;
	mpz_init(roundspt);

	mpz_cdiv_q_ui(roundspt, state.rounds, state.threads);

	/* Current offset */
	mpz_t offset;
	mpz_init(offset);

	/* Create the threads */
	for (int thread = 0; thread < state.threads; ++thread)
	{
		struct thread_info *info = &tinfos[thread];
		info->algorithm = state.algorithm;

		mpz_init(info->start);
		mpz_init(info->rounds);
		mpq_init(info->result);
	
		mpz_set(info->rounds, roundspt);
		mpz_set(info->start, offset);

		pthread_create(&info->thread_id, &pattr, (void*(*)(void*))thread_main, info);

		mpz_add(offset, offset, roundspt);
	}

	mpq_t result;
	mpq_init(result);

	/* Wait for the threads */
	for (int thread = 0; thread < state.threads; ++thread)
	{
		void *retval;
		pthread_join(tinfos[thread].thread_id, &retval);
		if (retval != PTHREAD_CANCELED)
			mpq_add(result, result, tinfos[thread].result);
	}

	print_mpq(result, state.digits, state.base);
}

