#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mpl.h"
#include "mpl_common.h"

extern void mpl_canonicalize(mpl_int *a);

int
_mpl_sqr_karatsuba(mpl_int *c, const mpl_int *x)
{
	mpl_int a2, b2, tmp;
	mpl_int a, b;
	unsigned int n;
	int top, rc;

	top = x->top;

	if ((rc = mpl_initv(&a2, &b2, &tmp, NULL)) != MPL_OK)
		return rc;

	/* Select half digits of the smallest number. */
	n = (top + 1) >> 1;

	/* Set up intermediate values used to calculate partial multiplications. */
	b.dig = x->dig;
	b.sign = MPL_SIGN_POS;
	b.top = n-1;
	b.alloc = 0;

	a.dig = x->dig + n;
	a.sign = MPL_SIGN_POS;
	a.top = top - n;
	a.alloc = 0;

	/* a^2 and b^2 */
	rc = mpl_sqr(&a2, &a);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_sqr(&b2, &b);
	if (rc != MPL_OK)
		goto err;

	/* tmp = (a+b)^2 - a^2 - b^2 */
	rc = mpl_add(&tmp, &a, &b);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_sqr(&tmp, &tmp);
	if (rc != MPL_OK)
		goto err;
	
	rc = mpl_sub(&tmp, &tmp, &a2);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_sub(&tmp, &tmp, &b2);
	if (rc != MPL_OK)
		goto err;

	/* c = a^2*2^2n + ((a + b)^2 - (a^2 + b^2))*2^n + b^2 */
	rc = mpl_shl(&a2, (n*MPL_INT_BITS));
	if (rc != MPL_OK)
		goto err;

	rc = mpl_add(c, &a2, &tmp);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_shl(c, (n*MPL_INT_BITS));
	if (rc != MPL_OK)
		goto err;

	rc = mpl_add(c, c, &b2);
	if (rc != MPL_OK)
		goto err;

	rc = MPL_OK;
err:
	mpl_clearv(&a2, &b2, &tmp, NULL);
	return rc;
}

int
_mpl_sqr_school(mpl_int *c, const mpl_int *a)
{
	mpl_int tmpint;
	_mpl_long_t w;
	_mpl_int_t *ap, *tmp;
	_mpl_int_t dig;
	int i, j, n, rc;
	int top;

	top = a->top;
	n = (top + 1) << 1;

	if ((rc = mpl_init(&tmpint)) != MPL_OK)
		return rc;

	--n;
	top = a->top;

	for (i = 0; i <= top; i++) {

		w = 0;
		ap = a->dig + i;
		dig = *ap++;

		/* Add square first. */
		tmp = tmpint.dig + (i << 1);
		w = (((_mpl_long_t)dig) * ((_mpl_long_t)dig)) + (_mpl_long_t)*tmp;
		*tmp++ = w & MPL_INT_MASK;
		w >>= MPL_INT_BITS;

		/* Next products have a factor of two. */
		for (j = i; j < top; j++) {
			w += (((_mpl_long_t)*ap++ * (_mpl_long_t)dig) << 1) + (_mpl_long_t)*tmp;
			*tmp++ = w & MPL_INT_MASK;
			w >>= MPL_INT_BITS;
		}

		/* Final carry. */
		*tmp = w;
	}

	tmpint.top = n;

	if ((rc = mpl_copy(c, &tmpint)) != MPL_OK)
		goto err_copy;

	mpl_canonicalize(c);
	rc = MPL_OK;

err_copy:
	mpl_clear(&tmpint);
	return rc;
}

int
_mpl_sqr_comba(mpl_int *c, const mpl_int *a)
{
	_mpl_int_t tmp[MPL_COMBA_STACK], *cp;
	_mpl_long_t w;
	int i, n;
	int rc, top, ctop;

	top = a->top;
	n = (top + 1) << 1;

	if ((rc = mpl_ensure(c, n)) != MPL_OK)
		return rc;

	/* Turn n into maximum number of iterations. */
	--n;

	/* Reset accumulator. */
	w = 0;

	for (i = 0; i < n; i++) {
		int begin, end;
		_mpl_int_t *p, *p2;

		tmp[i] = 0;
		begin = (i <= top) ? 0 : i - top;
		end = (i <= top) ? i : top;

		p = a->dig + begin;
		p2 = a->dig + end;

		/* Add two multiples of digits to accumulator. */
		while (p < p2)
			w += ((_mpl_long_t)(*p++) * (_mpl_long_t)(*p2--)) << 1;

		/* When i is even, add square. */
		if ((i & 0x01) == 0)
			w += (_mpl_long_t)(*p) * (_mpl_long_t)(*p);

		/* Store result. */
		tmp[i] = w & (_mpl_long_t)MPL_INT_MASK;

		/* Save carry for the next iteration. */
		w >>= MPL_INT_BITS;
	}

	/* Store final carry or 0 if none. */
	tmp[i] = w;

	cp = c->dig;
	ctop = c->top;
	c->top = n;

	/* Copy result. */
	for (i = 0; i <= n; i++)
		*cp++ = tmp[i];

	/* Zero unused digits in c. */
	for (; i <= ctop; i++)
		*cp++ = 0;

	mpl_canonicalize(c);

	return MPL_OK;
}

int
mpl_sqr(mpl_int *c, const mpl_int *x)
{
	int rc;

	if ((x->top+1) >= MPL_KARATSUBA_CUTOFF)
		rc = _mpl_sqr_karatsuba(c, x);
	else if (((x->top+1) << 1) <= MPL_COMBA_STACK &&
		  x->top+1 <= MPL_COMBA_DEPTH)
		rc = _mpl_sqr_comba(c, x);
	else
		rc = _mpl_sqr_school(c, x);

	/* squaring always gives positive sign */
	c->sign = MPL_SIGN_POS;
	
	return rc;
}

