#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "mpl.h"
#include "mpl_common.h"

int
mpl_primality_miller_rabin(const mpl_int *m, int r,
			  int (*rnd)(void *buf, size_t size, void *rndctx),
			  void *rndctx)
{
	mpl_int m_minus_one, one;
	mpl_int m_mod, a, t, x, mu;
	int i, rc, s, n;

	if (rnd == NULL || r == 0 || mpl_iszero(m))
		return MPL_ERR;

	rc = mpl_initv(&m_minus_one, &one, &a, &t, &x, &mu, NULL);
	if (rc != MPL_OK)
		return rc;
	/* m_mod = |m| */
	m_mod.dig = m->dig;
	m_mod.top = m->top;
	m_mod.sign = MPL_SIGN_POS;
	m_mod.alloc = 0;

	/* one = 1, m_minus_one = |m| - 1 */
	mpl_set_one(&one);

	rc = mpl_sub(&m_minus_one, &m_mod, &one);
	if (rc != MPL_OK)
		goto out;

	/* Prepare Barrett reduction precalculated constant. */
	rc = mpl_reduce_barrett_setup(&mu, &m_mod);
	if (rc != MPL_OK)
		goto out;

	/* Determine power of two used to construct |m|-1 = 2^s * t. */
	s = 0;
	n = m_minus_one.top + 1;

	for (i = 0; i < n; i++) {
		_mpl_int_t dig;
		int cnt;

		dig = m_minus_one.dig[i];

		for (cnt = 0; cnt < MPL_INT_BITS; cnt++) {
			if (dig & 0x1)
				goto brk_loop;
			dig >>= 1;
			++s;
		}
	}

brk_loop:

	/* The following condition is a paranoidal check. */
	if (i >= n) {
		rc = MPL_ERR;
		goto out;
	}

	/* m_minus_one = 2^s * t */
	rc = mpl_copy(&t, &m_minus_one);
	if (rc != MPL_OK)
		goto out;

	if (s > 0) {
		rc = mpl_shr(&t, s);
		if (rc != MPL_OK)
			goto out;
	}

	/* Number of random bytes one digit less than m. */
	n = (m->top * MPL_INT_BITS) / CHAR_BIT;

	for (i = 0; i < r; i++) {
		int j;

		//rc = mpl_random(&a, n, rnd, rndctx);
		rc = mpl_rand_below(&a, &m_minus_one, rnd, rndctx);
		if (rc != MPL_OK)
			goto out;

		rc = mpl_mod_exp(&x, &a, &t, &m_mod);
		if (rc != MPL_OK)
			goto out;

		if (mpl_cmp(&x, &one) == MPL_CMP_EQ)
			continue;

		if (mpl_cmp(&x, &m_minus_one) == MPL_CMP_EQ)
			continue;

		for (j = 1; j < s; j++) {
			
			rc = mpl_sqr(&x, &x);
			if (rc != MPL_OK)
				goto out;

			rc = mpl_reduce_barrett(&x, &x, &m_mod, &mu);
			if (rc != MPL_OK)
				goto out;

			if (mpl_isone(&x)) {
				rc = MPL_COMPOSITE;
				goto out;
			}

			if (mpl_cmp(&x, &m_minus_one) == MPL_CMP_EQ)
				break;
		}

		if (j >= s) {
			rc = MPL_COMPOSITE;
			goto out;
		}
	}

	rc = MPL_OK;
out:
	mpl_clearv(&m_minus_one, &one, &a, &t, &x, &mu, NULL);
	return rc;
}

