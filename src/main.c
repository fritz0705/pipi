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
#include <gmp.h>
#include <stdlib.h>
#include <string.h>

#ifndef ROUND_MULT
#	define ROUND_MULT 1
#endif

#ifndef PRINT_BASE
#	define PRINT_BASE 10
#endif

#ifdef WITH_THREADING
#	ifndef THREADS
#		define THREADS 4
#	endif
#	include <pthread.h>
struct thread_info {
	pthread_t thread_id;
	int       thread_num;
	mpz_t k;
	mpz_t rounds;
	mpq_t result;
};
#endif

/* Do something like: (a / (8*k +b)) */
static void get_bbp_part(mpz_t k, unsigned long a, unsigned long b, mpq_t
		result)
{
	/* Integer 8 */
	mpz_t int_8;
	mpz_init_set_ui(int_8, 8);

	/* Result of 8 * k */
	mpz_t k_mul_8;
	mpz_init(k_mul_8);

	mpz_mul(k_mul_8, k, int_8);

	/* Free the 8 */
	mpz_clear(int_8);

	/* Get GMP-representation of b */
	mpz_t int_b;
	mpz_init_set_ui(int_b, b);

	/* Result of (8 * k + b) */
	mpz_t k_mul_8_add_int_b;
	mpz_init(k_mul_8_add_int_b);

	mpz_add(k_mul_8_add_int_b, k_mul_8, int_b);

	mpz_clear(k_mul_8);
	mpz_clear(int_b);

	/* Get rational representation of c = (8 * k + b) */
	mpq_t rat_c;
	mpq_init(rat_c);
	mpq_set_z(rat_c, k_mul_8_add_int_b);

	mpz_clear(k_mul_8_add_int_b);

	/* Get rational representation of a */
	mpq_t rat_a;
	mpq_init(rat_a);
	mpq_set_ui(rat_a, a, 1);

	/* Calculate */

	mpq_div(result, rat_a, rat_c);

	mpq_clear(rat_a);
	mpq_clear(rat_c);
}

static void get_bbp_pow(mpz_t k, mpq_t result)
{
	/* Get integer values of 16 and 0 */
	mpz_t int_16, int_0;
	mpz_init(int_16);
	mpz_init(int_0);

	mpz_set_ui(int_16, 16);
	mpz_set_ui(int_0, 0);

	mpz_t pow_res;
	mpz_init(pow_res);
	
	unsigned int k_int = mpz_get_ui(k);
	mpz_pow_ui(pow_res, int_16, k_int);

	mpz_clear(int_16);
	mpz_clear(int_0);

	/* Get rational values of pow_res and 1 */
	mpq_t rat_1, rat_pow_res;
	mpq_init(rat_1);
	mpq_init(rat_pow_res);

	mpq_set_z(rat_pow_res, pow_res);
	mpz_clear(pow_res);
	mpq_set_ui(rat_1, 1, 1);

	mpq_div(result, rat_1, rat_pow_res);
	mpq_clear(rat_1);
	mpq_clear(rat_pow_res);
}

static void bbp_round(mpz_t k, mpq_t result)
{
	mpq_t pow_part; mpq_init(pow_part); get_bbp_pow(k, pow_part);
	mpq_t add1_part; mpq_init(add1_part); get_bbp_part(k, 4, 1, add1_part);
	mpq_t add2_part; mpq_init(add2_part); get_bbp_part(k, 2, 4, add2_part);
	mpq_t add3_part; mpq_init(add3_part); get_bbp_part(k, 1, 5, add3_part);
	mpq_t add4_part; mpq_init(add4_part); get_bbp_part(k, 1, 6, add4_part);

	/* add1 = add1_part - add2_part */
	mpq_t add1;
	mpq_init(add1);
	mpq_sub(add1, add1_part, add2_part);

	mpq_clear(add1_part);
	mpq_clear(add2_part);

	/* add2 = add1 - add3_part */
	mpq_t add2;
	mpq_init(add2);
	mpq_sub(add2, add1, add3_part);

	mpq_clear(add3_part);
	mpq_clear(add1);

	/* add3 = add2 - add4_part */
	mpq_t add3;
	mpq_init(add3);
	mpq_sub(add3, add2, add4_part);
	
	mpq_clear(add4_part);
	mpq_clear(add2);

	mpq_mul(result, pow_part, add3);
	mpq_clear(add3);
	mpq_clear(pow_part);
}

static void bbp_loop(mpz_t _k, mpz_t rounds, mpq_t result)
{
	mpz_t k;
	mpz_init_set(k, _k);

	mpz_t i;
	mpz_init_set_ui(i, 0);

	while (mpz_cmp(i, rounds) < 0)
	{
		mpq_t round;
		mpq_init(round);

		bbp_round(k, round);
		mpq_add(result, result, round);
		mpq_clear(round);

		mpz_add_ui(i, i, 1);
		mpz_add_ui(k, k, 1);
	}
}

static void print_mpq(mpq_t num, mpz_t digits)
{
	mpz_t numer;
	mpz_t denum;
	mpz_init(numer);
	mpz_init(denum);

	mpq_get_num(numer, num);
	mpq_get_den(denum, num);

	mpz_t int_base;
	mpz_init_set_ui(int_base, PRINT_BASE);

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

	mpz_out_str(stdout, PRINT_BASE, result);
	printf("\n");

	mpz_clear(int_base);
	mpz_clear(int_0);
	mpz_clear(mult);
	mpz_clear(new_numer);
	mpz_clear(result);
	mpz_clear(numer);
	mpz_clear(denum);
}

#ifdef WITH_THREADING
static void *thread_main(struct thread_info *info)
{
	bbp_loop(info->k, info->rounds, info->result);
	return NULL;
}
#endif

/* Usage: pipi [DIGITS] */
int main(int argc, char **argv)
{
	mpz_t digits;
	mpz_init(digits);

	mpz_set_ui(digits, 32);
	if (argc >= 2)
		for (int i = 1; i < argc; ++i)
			if (strcmp(argv[i], "--"))
				mpz_set_str(digits, argv[i], 10);

	mpz_t rounds;
	mpz_init(rounds);
#if 1
	{
		mpz_t int_mult;
		mpz_init_set_ui(int_mult, ROUND_MULT);

		mpz_mul(rounds, int_mult, digits);
		mpz_clear(int_mult);
	}
#endif

#ifndef WITH_THREADING
	mpz_t k;
	mpz_init_set_ui(k, 0);

	mpq_t result;
	mpq_init(result);
	bbp_loop(k, rounds, result);

	print_mpq(result, digits);
	mpq_clear(result);
#else
	mpz_t rounds_per_thread;
	mpz_init(rounds_per_thread);
	mpz_t k;
	mpz_init(k);
	pthread_attr_t attr;
	pthread_attr_init(&attr);

	mpz_cdiv_q_ui(rounds_per_thread, rounds, THREADS);

	struct thread_info infos[THREADS];
	for (int thread = 0; thread < THREADS; ++thread)
	{
		struct thread_info *info = &infos[thread];

		mpz_init(info->k);
		mpz_init(info->rounds);
		mpz_set(info->k, k);
		mpz_set(info->rounds, rounds_per_thread);
		mpq_init(info->result);

		int s = pthread_create(&info->thread_id, &attr, (void *(*)(void *))thread_main, info);
		if (s != 0)
			return 0;

		mpz_add(k, k, rounds_per_thread);
	}

	mpq_t result;
	mpq_init(result);
	for	(int thread = 0; thread < THREADS; ++thread)
	{
		void *retval;
		pthread_join(infos[thread].thread_id, &retval);
		if (retval != PTHREAD_CANCELED)
			mpq_add(result, result, infos[thread].result);
	}
	
	print_mpq(result, digits);
#endif
}
