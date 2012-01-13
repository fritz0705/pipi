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

#include "bbp.h"

/* Components are terms in the form (a / (8*k + b)) where a and b are natural numbers
 * A BBP formular consists of at least one component, so this function is useful
 * to reduce the code */
void bbp_component(mpz_t k, unsigned long a, unsigned long b, mpq_t result)
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

/* A BBP pow is a term like (a ^ (n*b) / c ^ n), where a, b and c are natural numbers.
 * Here I implement a bbp pow with a = 1, b = 0 and c = 16
 */
void bbp_pow(mpz_t k, mpq_t result)
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

void bbp_round(mpz_t k, mpq_t result)
{
	mpq_t pow_component; mpq_init(pow_component); bbp_pow(k, pow_component);
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

void bbp_loop(mpz_t _k, mpz_t rounds, mpq_t result)
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