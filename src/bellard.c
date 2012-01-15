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
#include "bellard.h"
#include "bbp.h"

static int bellard_is_negative(mpz_t n)
{
	mpz_t result;
	mpz_init(result);

	mpz_mod_ui(result, n, 2);

	int retval = mpz_cmp_ui(result, 0) != 0;
	mpz_clear(result);

	return retval;
}

static void bellard_pi_round(mpz_t k, mpq_t result)
{
	mpq_t _interm_1;
	mpq_init(_interm_1);

	bbp_component(k, 256, 1, 10, _interm_1);
	mpq_add(result, result, _interm_1);

	bbp_component(k, 32, 1, 4, _interm_1);
	mpq_sub(result, result, _interm_1);

	bbp_component(k, 1, 3, 4, _interm_1);
	mpq_sub(result, result, _interm_1);

	bbp_component(k, 64, 3, 10, _interm_1);
	mpq_sub(result, result, _interm_1);

	bbp_component(k, 4, 5, 10, _interm_1);
	mpq_sub(result, result, _interm_1);

	bbp_component(k, 4, 7, 10, _interm_1);
	mpq_sub(result, result, _interm_1);

	bbp_component(k, 1, 9, 10, _interm_1);
	mpq_add(result, result, _interm_1);

	mpz_t pow_denum;
	mpz_init(pow_denum);

	mpz_ui_pow_ui(pow_denum, 2, 10 * mpz_get_ui(k));

	mpz_t pow_num;
	mpz_init(pow_num);
	if (bellard_is_negative(k))
		mpz_set_si(pow_num, -1);
	else
		mpz_set_si(pow_num, 1);

	mpq_set_num(_interm_1, pow_num);
	mpq_set_den(_interm_1, pow_denum);

	mpq_mul(result, result, _interm_1);

	mpz_clear(pow_num);
	mpz_clear(pow_denum);
	mpq_clear(_interm_1);
}

void bellard_pi(mpz_t start, mpz_t rounds, mpq_t result)
{
	/* get 1 / 2**6 */
	mpq_t _const_1;
	mpq_init(_const_1);
	mpq_set_ui(_const_1, 1, 64);

	mpz_t k;
	mpz_init_set(k, start);

	mpz_t i;
	mpz_init_set_ui(i, 0);

	while (mpz_cmp(i, rounds) < 0)
	{
		mpq_t round;
		mpq_init(round);

		bellard_pi_round(k, round);
		mpq_add(result, result, round);
		mpq_clear(round);

		mpz_add_ui(i, i, 1);
		mpz_add_ui(k, k, 1);
	}

	mpq_mul(result, result, _const_1);

	mpq_clear(_const_1);
}
