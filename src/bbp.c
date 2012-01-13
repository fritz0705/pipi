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

/* A BBP pow is a term like (a ^ (k*b) / c ^ k), where a, b and c are natural numbers.
 */
void bbp_pow(mpz_t k, mpq_t result, unsigned int a, unsigned int b, unsigned int c)
{
	unsigned int k_ui = mpz_get_ui(k);

	mpz_t int_c;
	mpz_init_set_ui(int_c, c);

	mpz_t denum;
	mpz_init(denum);

	mpz_pow_ui(denum, int_c, k_ui);

	unsigned int num_exp = k_ui * b;
	mpz_t int_a;
	mpz_init_set_ui(int_a, a);

	mpz_t numer;
	mpz_init(numer);

	mpz_pow_ui(numer, int_a, num_exp);

	mpq_set_num(result, numer);
	mpq_set_den(result, denum);

	mpz_clear(numer);
	mpz_clear(denum);
	mpz_clear(int_a);
	mpz_clear(int_c);
}

#if 0
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
#endif
