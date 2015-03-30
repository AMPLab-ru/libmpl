#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mpl.h"
#include "mpl_common.h"

extern void mpl_canonicalize(mpl_int *a);

int
mpl_div(mpl_int *q, mpl_int *r, const mpl_int *y, const mpl_int *x)
{
	mpl_int res, u, v, tmp;
	_mpl_int_t dig;
	int norm, cnt, rc;
	int ytop, ysign;
	int xtop, xsign;
	int i, n, t, sign;

	if (mpl_iszero(x))
		return MPL_ERR;

	if (mpl_abs_cmp(x, y) == MPL_CMP_GT) {
		if (r != NULL) {
			if ((rc = mpl_copy(r, y)) != MPL_OK)
				return rc;
		}
		if (q != NULL)
			mpl_zero(q);
		return MPL_OK;
	}

	if ((rc = mpl_initv(&u, &v, &tmp, &res, NULL)) != MPL_OK)
		return rc;

	if (q == NULL)
		q = &res;

	if ((rc = mpl_copy(&u, y)) != MPL_OK)
		goto err;

	if ((rc = mpl_copy(&v, x)) != MPL_OK)
		goto err;

	u.sign = MPL_SIGN_POS;
	v.sign = MPL_SIGN_POS;

	ytop = y->top;
	xtop = x->top;
	ysign = y->sign;
	xsign = x->sign;

	/* Guess about the result sign. */
	sign = (ysign == xsign) ? MPL_SIGN_POS: MPL_SIGN_NEG;

	/* Reserve memory in quotient and remainder. */
	if ((rc = mpl_ensure(q, ytop - xtop + 1)) != MPL_OK)
		goto err;

	if (r != NULL) {
		if ((rc = mpl_ensure(r, xtop + 1)) != MPL_OK)
			goto err;
	}

	/* Count bits of the topmost digit of divider. */
	dig = x->dig[xtop];
	cnt = 0;
	while (dig > 0) {
		dig >>= 1;
		++cnt;
	}

	/* Normalize divider and dividend. */
	if (cnt < MPL_INT_BITS) {
		norm = (MPL_INT_BITS) - cnt;
		if ((rc = mpl_shl(&u, norm)) != MPL_OK)
			goto err;
		if ((rc = mpl_shl(&v, norm)) != MPL_OK)
			goto err;
	} else {
		norm = 0;
	}

	/* Setup quotient properties known in advance. */
	mpl_zero(q);
	q->top = ytop - xtop;
	q->sign = sign;

	for (;;) {
		n = u.top;
		t = v.top;
		if (n < t)
			break;
		cnt = (n - t) * MPL_INT_BITS;

		if ((rc = mpl_shl(&v, cnt)) != MPL_OK)
			goto err;

		if (mpl_abs_cmp(&u, &v) != MPL_CMP_LT) {
			do {
				++q->dig[n-t];
				if ((rc = mpl_sub(&u, &u, &v)) != MPL_OK)
					goto err;
			} while (mpl_abs_cmp(&u, &v) != MPL_CMP_LT);

			if ((rc = mpl_shr(&v, cnt)) != MPL_OK)
				goto err;
		} else {
			if ((rc = mpl_shr(&v, cnt)) != MPL_OK)
				goto err;
			break;
		}
	}

	for (i = n; i >= t+1; i--) {
		_mpl_int_t tmpint[3];
		_mpl_int_t qcap;
		mpl_int tmp2;
		int qpos;

		/* Skip empty dividend digits. */
/*		if (i > u.top)
			continue;
*/
		/* Precalculate quotient digit position. */
		qpos = i - t - 1;

		/* First approximation to quotient. */
		if (u.dig[i] == v.dig[t]) {
			qcap = MPL_INT_BASE-1;
		} else {
			_mpl_long_t tmp;
			tmp  = (_mpl_long_t) u.dig[i] << MPL_INT_BITS;
			tmp |= (_mpl_long_t) u.dig[i-1];
			tmp /= (_mpl_long_t) v.dig[t];
			if (tmp >= MPL_INT_BASE)
				tmp = MPL_INT_BASE-1;
			qcap = tmp & MPL_INT_MASK;
		}

		++qcap;

		/* 
		 * Refine quotient approximation using 3 digits of dividend and
		 * 2 digits of divisor. This refinement ensures that quotient
		 * will be less then or equal to (q+1).
		 */
		do {
			--qcap;
			tmp.top = 1;
			tmp.sign = MPL_SIGN_POS;
			tmp.dig[1] = v.dig[t];
			tmp.dig[0] = t > 0 ? v.dig[t-1] : 0;

			tmp2.top = 2;
			tmp2.dig = tmpint;
			tmp2.sign = MPL_SIGN_POS;
			tmpint[2] = u.dig[i];
			tmpint[1] = i > 0 ? u.dig[i-1] : 0;
			tmpint[0] = i > 1 ? u.dig[i-2] : 0;

			if ((rc = mpl_mul_dig(&tmp, &tmp, qcap)) != MPL_OK)
				goto err;

			++cnt;
		} while (mpl_abs_cmp(&tmp, &tmp2) == MPL_CMP_GT);
	
		/* u = u - qv*b^{i-t-1} */

		if ((rc = mpl_mul_dig(&tmp, &v, qcap)) != MPL_OK)
			goto err;

		if ((rc = mpl_shl(&tmp, qpos*MPL_INT_BITS)) != MPL_OK)
			goto err;

		if ((rc = mpl_sub(&u, &u, &tmp)) != MPL_OK)
			goto err;

		if (u.sign == MPL_SIGN_NEG) {
			/* Fix divisor if the quotient approximation was by one bigger. */
			--qcap;
			if ((rc = mpl_mul_dig(&tmp, &v, qcap)) != MPL_OK)
				goto err;
			if ((rc == mpl_shl(&tmp, qpos*MPL_INT_BITS)) != MPL_OK)
				goto err;	
			if ((rc = mpl_add(&u, &u, &tmp)) != MPL_OK)
				goto err;
		}

		/* Store quotient digit. */
		q->dig[qpos] = qcap;
	}

	if (r != NULL) {
		if ((rc = mpl_copy(r, &u)) != MPL_OK)
			goto err;

		if (norm > 0)
			mpl_shr(r, norm);
		r->sign = ysign;
	}

	mpl_canonicalize(q);
	rc = MPL_OK;
err:
	mpl_clearv(&u, &v, &tmp, &res, NULL);
	return rc;
}

