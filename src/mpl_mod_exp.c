#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mpl.h"
#include "mpl_common.h"

#define MPL_MODEXP_STACK	128

int
mpl_mod_exp(mpl_int *c, const mpl_int *a, const mpl_int *y, const mpl_int *b)
{

	mpl_int w[MPL_MODEXP_STACK];
	mpl_int e, s, mu, z;
	_mpl_int_t *dp, buffer;
	int i, k, nbits, rc;
	int do_single;
	unsigned int mask, x, n, tmp, cnt;

	if ((rc = mpl_initv(&e, &s, &mu, &z, NULL)) != MPL_OK)
		return rc;

	if (mpl_isneg(y)) {
		if ((rc = mpl_mod_inv(&z, a, b)) != MPL_OK)
			goto end;
	} else {
		if ((rc = mpl_copy(&z, a)) != MPL_OK)
			goto end;
	}

	n = mpl_nr_bits(y);

	if (n <= 7)
		k = 2;
	else if (n <= 36)
		k = 3;
	else if (n <= 140)
		k = 4;
	else if (n <= 450)
		k = 5;
	else if (n <= 1303)
		k = 6;
	else if (n <= 3529)
		k = 7;
	else
		k = 8;

	cnt = 0;

	for (i = 0; i < 1 << (k-1); i++) {
		rc = mpl_init(&w[i]);
		if (rc != MPL_OK) {
			for (i = 0; i < cnt; i++)
				mpl_clear(&w[i]);
			goto end;
		}
		++cnt;
	}

	/* e = a */
	rc = mpl_copy(&e, &z);
	if (rc != MPL_OK)
		goto err;

	/* c = 1 */
	mpl_set_one(&s);

	/* prepare reduction constant */
	rc = mpl_reduce_barrett_setup(&mu, b);
	if (rc != MPL_OK)
		goto err;

	/* e = a^{2^(k-1)} */
	for (i = 0; i < k-1; i++) {
		rc = mpl_sqr(&e, &e);
		if (rc != MPL_OK)
			goto err;
		rc = mpl_reduce_barrett(&e, &e, b, &mu);
		if (rc != MPL_OK)
			goto err;
	}

	/* Now fill precomputed table. */
	rc = mpl_copy(&w[0], &e);
	if (rc != MPL_OK)
		goto err;

	for (i = 1; i < 1 << (k-1); i++) {
		/* w[i] = (w[i-1] * a) mod b */
		rc = mpl_mul(&w[i], &w[i-1], &z);
		if (rc != MPL_OK)
			goto err;
		rc = mpl_reduce_barrett(&w[i], &w[i], b, &mu);
		if (rc != MPL_OK)
			goto err;
	}
	
	buffer = nbits = do_single = cnt = 0;

	/* Count bits of the topmost MP integer digit. */
	dp = y->dig + y->top;
	tmp = *dp;
	for (cnt = 0; tmp > 0; cnt++)
		tmp >>= 1;
	nbits = cnt;
	buffer = *dp--;

	/* Precalculated window mask. */
	mask = (1 << k) - 1;

	while (n > 0) {
		unsigned int left, xmask;

		if (nbits == 0) {
			buffer = *dp--;
			nbits = MPL_INT_BITS;
		}

		/* Check most significant bit of the bit buffer. */
		if ((buffer & (1 << (nbits-1))) == 0) {
			/* c = c^2 mod b */
			rc = mpl_sqr(&s, &s);
			if (rc != MPL_OK)
				goto err;

			rc = mpl_reduce_barrett(&s, &s, b, &mu);
			if (rc != MPL_OK)
				goto err;

			--nbits;
			--n;
			continue;
		}

		if (nbits >= k) {
			/* We have enough bits in the buffer to fill window. */
			x = (buffer & (mask << (nbits-k))) >> (nbits-k);
			nbits -= k;
			n -= k;
		} else {
			/* Less then k bits left in the buffer. */
			left = k;

			/* Consume remaining bits from the buffer. */
			x = buffer & ((1 << nbits)-1);
			left -= nbits;

			/* 
			 * Fallback to single-bit exponentiation if we can't
			 * get enough bits to form a k-bit window.
			 */
			if (n == nbits) {
				do_single = 1;
				break;
			} else if (n < k) {
				n -= nbits;
				buffer = *dp--;
				x <<= n;
				x |= buffer;
				do_single = 1;
				break;
			}

			buffer = *dp--;
			xmask = (1 << left)-1;
			x <<= left;
			x |= (buffer & (xmask << (MPL_INT_BITS - left))) >> (MPL_INT_BITS - left);
			nbits = MPL_INT_BITS - left;
			n -= k;
		}

		/* c = c^2k mod b  */
		for (i = 0; i < k; i++) {
			rc = mpl_sqr(&s, &s);
			if (rc != MPL_OK)
				goto err;
			rc = mpl_reduce_barrett(&s, &s, b, &mu);
			if (rc != MPL_OK)
				goto err;
		}

		/* c = c * a[x] */
		tmp = (1 << (k-1))-1;
		rc = mpl_mul(&s, &s, &w[x & tmp]);
		if (rc != MPL_OK)
			goto err;

		rc = mpl_reduce_barrett(&s, &s, b, &mu);
		if (rc != MPL_OK)
			goto err;
	}

	if (do_single) {

		while (n > 0) {
			/* c = c^2 */
			rc = mpl_sqr(&s, &s);
			if (rc != MPL_OK)
				goto err;
			rc = mpl_reduce_barrett(&s, &s, b, &mu);
			if (rc != MPL_OK)
				goto err;

			if (x & (1 << (n-1))) {
				/* c = c * a mod b*/
				rc = mpl_mul(&s, &s, &z);
				if (rc != MPL_OK)
					goto err;
				rc = mpl_reduce_barrett(&s, &s, b, &mu);
				if (rc != MPL_OK)
					goto err;
			}
			--n;
		}
	}

	rc = mpl_copy(c, &s);
	if (rc != MPL_OK)
		goto err;

	rc = MPL_OK;

err:
	for (i = 0; i < (1 << (k-1)); i++)
		mpl_clear(&w[i]);
end:
	mpl_clearv(&s, &e, &mu, &z, NULL);

	return rc;
}

static int
mpl_mod_mul(mpl_int *c, const mpl_int *a, const mpl_int *b, const mpl_int *m)
{
	mpl_int tmp;
	int rc;

	rc = MPL_ERR;

	mpl_init(&tmp);

	if (mpl_mul(c, a, b) != MPL_OK)
		goto end;
	
	rc = mpl_div(&tmp, c, c, m);

end:
	mpl_clear(&tmp);
	return rc;
}

int
mpl_mod_exp_simple(mpl_int *c, const mpl_int *a, const mpl_int *y, const mpl_int *b)
{
	mpl_int res, tmp, u, v;
	int rc;

	rc = MPL_ERR;

	if ((rc = mpl_initv(&res, &tmp, &u, &v, NULL)) != MPL_OK)
		return rc;

	rc = mpl_copy(&u, a);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_copy(&v, y);
	if (rc != MPL_OK)
		goto err;
	
	mpl_set_one(&res);

	while(!mpl_iszero(&v)) {
		if (mpl_isodd(&v)) {
			mpl_mod_mul(&res, &res, &u, b);
			if (rc != MPL_OK)
				goto err;
		}
		
		rc = mpl_shr(&v, 1);
		if (rc != MPL_OK)
			goto err;
		
		rc = mpl_mod_mul(&u, &u, &u, b);
		if (rc != MPL_OK)
			goto err;
	}

	mpl_copy(c, &res);
	mpl_canonicalize(c);

	rc = MPL_OK;
err:
	mpl_clearv(&res, &tmp, &u, &v, NULL);
	return rc;
}

