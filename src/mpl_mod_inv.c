#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mpl.h"
#include "mpl_common.h"

int
mpl_mod_inv(mpl_int *c, const mpl_int *a, const mpl_int *b)
{
	mpl_int x1, x2, y1, y2;
	mpl_int x, y, u, v;
	int rc;

	/* Modulo can't be negative. */
	if (b->sign == MPL_SIGN_NEG)
		return MPL_ERR;

	/* 0 < a < b */
	if (mpl_iszero(a) || mpl_iszero(b))
		return MPL_ERR;

	if (mpl_abs_cmp(a, b) == MPL_CMP_GT)
		return MPL_ERR;

	/* Both a and b can't be even because then gcd(a, b) != 1. */
	if (mpl_iseven(a) && mpl_iseven(b))
		return MPL_ERR;
	
	if ((rc = mpl_initv(&x1, &x2, &y1, &y2, &u, &v, NULL)) != MPL_OK)
		return rc;

	x.sign = MPL_SIGN_POS;
	x.dig = a->dig;
	x.top = a->top;
	x.alloc = 0;

	y.sign = MPL_SIGN_POS;
	y.dig = b->dig;
	y.top = b->top;
	y.alloc = 0;

	rc = mpl_copy(&u, &x);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_copy(&v, &y);
	if (rc != MPL_OK)
		goto err;

	mpl_set_one(&x1);
	mpl_set_one(&y2);

	do { 
		while (mpl_iseven(&u)) {
			/* u = u / 2 */
			rc = mpl_shr(&u, 1);
			if (rc != MPL_OK)
				goto err;

			if (mpl_isodd(&x1) || mpl_isodd(&y1)) {
				/* x1 = x1 + y, y1 = y1 - x */
				rc = mpl_add(&x1, &x1, &y);
				if (rc != MPL_OK)
					goto err;
				rc = mpl_sub(&y1, &y1, &x);
				if (rc != MPL_OK)
					goto err;
			}

			/* x1 = x1 / 2, y1 = y1 / 2 */
			rc = mpl_shr(&x1, 1);
			if (rc != MPL_OK)
				goto err;

			rc = mpl_shr(&y1, 1);
			if (rc != MPL_OK)
				goto err;
		}

		while (mpl_iseven(&v)) {
			/* v = v / 2 */
			rc = mpl_shr(&v, 1);
			if (rc != MPL_OK)
				goto err;

			if (mpl_isodd(&x2) || mpl_isodd(&y2)) {
				/* x2 = x2 + y, y2 = y2 - x */
				rc = mpl_add(&x2, &x2, &y);
				if (rc != MPL_OK)
					goto err;
				rc = mpl_sub(&y2, &y2, &x);
				if (rc != MPL_OK)
					goto err;
			}

			/* x2 = x2 / 2 */
			rc = mpl_shr(&x2, 1);
			if (rc != MPL_OK)
				goto err;

			/* y2 = y2 / 2 */
			rc = mpl_shr(&y2, 1);
			if (rc != MPL_OK)
				goto err;
		}

		if (mpl_cmp(&u, &v) != MPL_CMP_LT) {
			/* u = u - v, x1 = x1 - x2, y1 = y1 - y2 */
			rc = mpl_sub(&u, &u, &v);
			if (rc != MPL_OK)
				goto err;
			rc = mpl_sub(&x1, &x1, &x2);
			if (rc != MPL_OK)
				goto err;
			rc = mpl_sub(&y1, &y1, &y2);
			if (rc != MPL_OK)
				goto err;
		} else {
			/* v = v - u, x2 = x2 - x1, y2 = y2 - y1 */
			rc = mpl_sub(&v, &v, &u);
			if (rc != MPL_OK)
				goto err;
			rc = mpl_sub(&x2, &x2, &x1);
			if (rc != MPL_OK)
				goto err;
			rc = mpl_sub(&y2, &y2, &y1);
			if (rc != MPL_OK)
				goto err;
		}

	} while (!mpl_iszero(&u) && !mpl_iszero(&v));

	if (mpl_iszero(&v)) {
		
		if (!mpl_isone(&u))
			return MPL_ERR;

		rc = mpl_copy(c, &x1);
		if (rc != MPL_OK)
			goto err;
	} else {
		
		if (!mpl_isone(&v))
			return MPL_ERR;
		
		rc = mpl_copy(c, &x2);
		if (rc != MPL_OK)
			goto err;
	}

	/* Result is congruent modulo b. */
	while (mpl_cmp(c, b) == MPL_CMP_GT) {
		if ((rc = mpl_sub(c, c, b)) != MPL_OK)
			goto err;
	}

	while (c->sign == MPL_SIGN_NEG) {
		if ((rc = mpl_add(c, c, b)) != MPL_OK)
			goto err;
	}

	rc = MPL_OK;
err:
	mpl_clearv(&x1, &x2, &y1, &y2, &u, &v, NULL);
	return rc;
}

