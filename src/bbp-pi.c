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

#include "bbp-pi.h"
#include "bbp.h"

static void bbp_pi_round(mpz_t k, mpq_t result)
{
	mpq_t pow_component; mpq_init(pow_component); bbp_pow(k, pow_component, 1, 0, 16);
	mpq_t add1_component; mpq_init(add1_component); bbp_component(k, 4, 1, add1_component);
	mpq_t add2_component; mpq_init(add2_component); bbp_component(k, 2, 4, add2_component);
	mpq_t add3_component; mpq_init(add3_component); bbp_component(k, 1, 5, add3_component);
	mpq_t add4_component; mpq_init(add4_component); bbp_component(k, 1, 6, add4_component);

	/* add1 = add1_component - add2_component */
	mpq_t add1;
	mpq_init(add1);
	mpq_sub(add1, add1_component, add2_component);

	mpq_clear(add1_component);
	mpq_clear(add2_component);

	/* add2 = add1 - add3_component */
	mpq_t add2;
	mpq_init(add2);
	mpq_sub(add2, add1, add3_component);

	mpq_clear(add3_component);
	mpq_clear(add1);

	/* add3 = add2 - add4_component */
	mpq_t add3;
	mpq_init(add3);
	mpq_sub(add3, add2, add4_component);
	
	mpq_clear(add4_component);
	mpq_clear(add2);

	mpq_mul(result, pow_component, add3);
	mpq_clear(add3);
	mpq_clear(pow_component);
}

void bbp_pi(mpz_t start, mpz_t rounds, mpq_t result)
{
	mpz_t k;
	mpz_init_set(k, start);

	mpz_t i;
	mpz_init_set_ui(i, 0);

	while (mpz_cmp(i, rounds) < 0)
	{
		mpq_t round;
		mpq_init(round);

		bbp_pi_round(k, round);
		mpq_add(result, result, round);
		mpq_clear(round);

		mpz_add_ui(i, i, 1);
		mpz_add_ui(k, k, 1);
	}
}
