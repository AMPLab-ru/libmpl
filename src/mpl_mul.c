#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include "mpl.h"
#include "mpl_common.h"

extern void mpl_canonicalize(mpl_int *a);

/* This function is useless and written just for fun. */
int
_mpl_mul_toom_three(mpl_int *c, const mpl_int *a, const mpl_int *b)
{
	mpl_int p, p0, p1, p2, p3, p4;
	mpl_int q, q0, q1, q2, q3, q4;
	mpl_int m0, m1, m2;
	mpl_int n0, n1, n2;
	mpl_int three;
	_mpl_int_t num;
	int n, top, atop, btop, nbits;
	int rc;

	rc = mpl_initv(&p, &p0 ,&p1, &p2, &p3, &p4, &q, &q0, &q1, &q2, &q3, &q4, NULL);
	if (rc != MPL_OK)
		return rc;

	num = 0;

	atop = a->top;
	btop = b->top;

	n = (atop > btop) ? atop : btop;
	n++;
	n = (n % 3 == 0) ? n/3 : n/3 + 1;
	top = n - 1;

	/* set up initial values */
	m0.sign  = MPL_SIGN_POS;
	m0.alloc = 0;
	m0.top   = (atop < top) ? atop : top;
	m0.dig   = a->dig;

	m1.sign  = MPL_SIGN_POS;
	m1.alloc = 0;
	if (atop < top) {
		m1.top = -1;
		m1.dig = &num;
	} else {
		m1.top = (atop < 2*top) ? atop - top : top;
		m1.dig = a->dig + n;
	}

	m2.sign  = MPL_SIGN_POS;
	m2.alloc = 0;
	if (atop < 2*top) {
		m2.top = -1;
		m2.dig = &num;
	} else {
		m2.top = (atop < 3*top) ? atop - (2*top) : top;
		m2.dig = a->dig + (2*n);
	}

	n0.sign  = MPL_SIGN_POS;
	n0.alloc = 0;
	n0.top   = (btop < top) ? btop : top;
	n0.dig   = b->dig;

	n1.sign  = MPL_SIGN_POS;
	n1.alloc = 0;
	if (btop < top) {
		n1.top = -1;
		n1.dig = &num;
	} else {
		n1.top = (btop < 2*top) ? btop - top : top;
		n1.dig = b->dig + n;
	}

	n2.sign  = MPL_SIGN_POS;
	n2.alloc = 0;
	if (btop < 2*top) {
		n2.top = -1;
		n2.dig = &num;
	} else {
		n2.top = (btop < 3*top) ? btop - (2*top) : top;
		n2.dig = b->dig + (2*n);
	}

	/* prepare polynomial coefficients for p */
	/* p = m0 + m2 */
	rc = mpl_add(&p, &m0, &m2);
	if (rc != MPL_OK)
		goto err;

	/* p0 = m0 */
	rc = mpl_copy(&p0, &m0);
	if (rc != MPL_OK)
		goto err;

	/* p1 = p + m1 */
	rc = mpl_add(&p1, &p, &m1);
	if (rc != MPL_OK)
		goto err;

	/* p2 = p - m1 */
	rc = mpl_sub(&p2, &p, &m1);
	if (rc != MPL_OK)
		goto err;

	/* p3 = (p2 + m2)*2 - m0 */
	rc = mpl_add(&p3, &p2, &m2);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_shl(&p3, 1);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_sub(&p3, &p3, &m0);
	if (rc != MPL_OK)
		goto err;

	/* p4 = m2 */
	rc = mpl_copy(&p4, &m2);
	if (rc != MPL_OK)
		goto err;

	/* prepare polynomial coefficients for q */
	/* q = n0 + n2 */
	rc = mpl_add(&q, &n0, &n2);
	if (rc != MPL_OK)
		goto err;

	/* q0 = n0 */
	rc = mpl_copy(&q0, &n0);
	if (rc != MPL_OK)
		goto err;

	/* q1 = q + n1 */
	rc = mpl_add(&q1, &q, &n1);
	if (rc != MPL_OK)
		goto err;

	/* q2 = q - n1 */
	rc = mpl_sub(&q2, &q, &n1);
	if (rc != MPL_OK)
		goto err;

	/* q3 = (q2 + n2)*2 - n0 */
	rc = mpl_add(&q3, &q2, &n2);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_shl(&q3, 1);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_sub(&q3, &q3, &n0);
	if (rc != MPL_OK)
		goto err;

	/* q4 = n2 */
	rc = mpl_copy(&q4, &n2);
	if (rc != MPL_OK)
		goto err;

	/* pointwise multiplication */
	/* q0 = q0 * p0 */
	rc = mpl_mul(&q0, &q0, &p0);
	if (rc != MPL_OK)
		goto err;

	/* q1 = q1 * p1 */
	rc = mpl_mul(&q1, &q1, &p1);
	if (rc != MPL_OK)
		goto err;

	/* q2 = q2 * p2 */
	rc = mpl_mul(&q2, &q2, &p2);
	if (rc != MPL_OK)
		goto err;

	/* q3 = q3 * p3 */
	rc = mpl_mul(&q3, &q3, &p3);
	if (rc != MPL_OK)
		goto err;

	/* q4 = q4 * p4 */
	rc = mpl_mul(&q4, &q4, &p4);
	if (rc != MPL_OK)
		goto err;

	/* interpolation */
	/* p0 = q0 */
	rc = mpl_copy(&p0, &q0);
	if (rc != MPL_OK)
		goto err;

	/* p4 = q4 */
	rc = mpl_copy(&p4, &q4);
	if (rc != MPL_OK)
		goto err;

	/* p3 = (q3 - q1)/3 */
	rc = mpl_sub(&p3, &q3, &q1);
	if (rc != MPL_OK)
		goto err;

	num = 3;

	three.sign  = MPL_SIGN_POS;
	three.alloc = 0;
	three.top   = 0;
	three.dig   = &num;

	rc = mpl_div(&p3, NULL, &p3, &three);
	if (rc != MPL_OK)
		goto err;

	/* p1 = (q1 - q2)/2 */
	rc = mpl_sub(&p1, &q1, &q2);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_shr(&p1, 1);
	if (rc != MPL_OK)
		goto err;

	/* p2 = q2 - q0 */
	rc = mpl_sub(&p2, &q2, &q0);
	if (rc != MPL_OK)
		goto err;

	/* p3 = (p2 - p3)/2 + 2*q4 */
	rc = mpl_sub(&p3, &p2, &p3);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_shr(&p3, 1);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_shl(&q4, 1);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_add(&p3, &p3, &q4);
	if (rc != MPL_OK)
		goto err;

	/* p2 = p2 + p1 - p4 */
	rc = mpl_add(&p2, &p2, &p1);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_sub(&p2, &p2, &p4);
	if (rc != MPL_OK)
		goto err;

	/* p1 = p1 - p3 */
	rc = mpl_sub(&p1, &p1, &p3);
	if (rc != MPL_OK)
		goto err;

	/* evaluate the result */
	mpl_zero(c);

	rc = mpl_copy(c, &p0);
	if (rc != MPL_OK)
		goto err;

	nbits = MPL_INT_BITS * n;

	rc = mpl_shl(&p1, nbits);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_add(c, c, &p1);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_shl(&p2, 2*nbits);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_add(c, c, &p2);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_shl(&p3, 3*nbits);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_add(c, c, &p3);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_shl(&p4, 4*nbits);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_add(c, c, &p4);
	if (rc != MPL_OK)
		goto err;

	c->sign = (a->sign == b->sign) ? MPL_SIGN_POS : MPL_SIGN_NEG;

	rc = MPL_OK;
err:
	mpl_clearv(&p, &p0 ,&p1, &p2, &p3, &p4, &q, &q0, &q1, &q2, &q3, &q4, NULL);

	return rc;
}

int
_mpl_mul_karatsuba(mpl_int *c, const mpl_int *a, const mpl_int *b)
{
	mpl_int z0, z2;
	mpl_int tmp1, tmp2;
	mpl_int x0, x1, y0, y1;
	unsigned int n;
	int atop, btop, rc;

	atop = a->top;
	btop = b->top;

	rc = mpl_initv(&z0, &z2, &tmp1, &tmp2, NULL);

	if (rc != MPL_OK)
		return rc;

	/* Select half digits of the smallest number. */
	n = ((atop > btop) ? (btop+1) : (atop+1)) >> 1;

	/* Set up intermediate values used to calculate partial multiplications. */
	x0.dig = a->dig;
	x0.sign = MPL_SIGN_POS;
	x0.top = n-1;
	x0.alloc = 0;

	x1.dig = a->dig + n;
	x1.sign = MPL_SIGN_POS;
	x1.top = atop - n;
	x1.alloc = 0;

	y0.dig = b->dig;
	y0.sign = MPL_SIGN_POS;
	y0.top = n-1;
	y0.alloc = 0;

	y1.dig = b->dig + n;
	y1.sign = MPL_SIGN_POS;
	y1.top = btop - n;
	y1.alloc = 0;

	/* calculate z2 = x1 * y1 */
	if ((rc = mpl_mul(&z2, &x1, &y1)) != MPL_OK)
		goto err;

	/* calculate z0 = x0 * y0 */
	if ((rc = mpl_mul(&z0, &x0, &y0)) != MPL_OK)
		goto err;

	/* calculate tmp1 = x1 + x0 and tmp2 = y1 + y0 */
	if ((rc = mpl_add(&tmp1, &x1, &x0)) != MPL_OK)
		goto err;

	if ((rc = mpl_add(&tmp2, &y1, &y0)) != MPL_OK)
		goto err;

	/* calculate z1 = (x1 + x0)*(y1 + y0) - z2 - z0, store result in tmp1 */
	if ((rc = mpl_mul(&tmp1, &tmp1, &tmp2)) != MPL_OK)
		goto err;
	if ((rc = mpl_sub(&tmp1, &tmp1, &z2)) != MPL_OK)
		goto err;
	if ((rc = mpl_sub(&tmp1, &tmp1, &z0)) != MPL_OK)
		goto err;

	/* exponentiate z2 to 2^2n and z1 (tmp1) to 2^n */
	if ((rc = mpl_shl(&z2, (n*MPL_INT_BITS) << 1)) != MPL_OK)
		goto err;
	if ((rc = mpl_shl(&tmp1, n*MPL_INT_BITS)) != MPL_OK)
		goto err;

	/* c = z0 + z1 + z2 */
	if ((rc = mpl_add(c, &z0, &tmp1)) != MPL_OK)
		goto err;

	if ((rc = mpl_add(c, c, &z2)) != MPL_OK)
		goto err;

	rc = MPL_OK;
err:
	mpl_clearv(&z0, &z2, &tmp1, &tmp2, NULL);
	
	return rc;
}

/* Multiplies c = |a| * |b|, |a| > |b| using Comba O(n^2) method. */
int
_mpl_mul_comba(mpl_int *c, const mpl_int *a, const mpl_int *b)
{
	_mpl_int_t tmp[MPL_COMBA_STACK];
	_mpl_int_t *ap, *bp, *cp;
	_mpl_long_t r;
	int i, ai, rc, bplus1;
	unsigned int amin, bmin, amax;
	int atop, btop, maxtop, oldtop;

	atop = a->top;
	btop = b->top;
	bplus1 = btop + 1;
	maxtop = atop + btop + 1;
	oldtop = c->top;

	if ((rc = mpl_ensure(c, maxtop+1)) != MPL_OK)
		return rc;

	c->top = maxtop;
	r = 0;

	for (i = 0, cp = tmp; i < maxtop; i++, cp++) {

		amin = (i >= bplus1) ? i-btop : 0;
		bmin = (i >= bplus1) ? btop : i;
		amax = (i > atop) ? atop : i;

		ap = a->dig + amin;
		bp = b->dig + bmin;

		*cp = 0;

		for (ai = amin; ai <= amax; ai++)
			r += (_mpl_long_t)(*ap++) * (_mpl_long_t)(*bp--);

		*cp = r & MPL_INT_MASK;
		r >>= MPL_INT_BITS;
	}

	*cp = r;

	ap = tmp;
	bp = c->dig;

	for (i = 0; i <= maxtop; i++)
		*bp++ = *ap++;

	for (; i <= oldtop; i++)
		*bp++ = 0;

	mpl_canonicalize(c);

	return MPL_OK;
}

/* 
 * Slightly modified Comba multiplication calculates upper part
 * of the result starting from digit.
 */
int
_mpl_mul_comba_upper(mpl_int *c, const mpl_int *a, const mpl_int *b, int digit)
{
	_mpl_int_t tmp[MPL_COMBA_STACK];
	_mpl_int_t *ap, *bp, *cp;
	_mpl_long_t r;
	int i, ai, rc, bplus1;
	unsigned int amin, bmin, amax;
	int atop, btop, maxtop, oldtop;

	atop = a->top;
	btop = b->top;
	bplus1 = btop + 1;
	maxtop = atop + btop + 1;
	oldtop = c->top;

	if ((rc = mpl_ensure(c, maxtop+1)) != MPL_OK)
		return rc;

	c->top = maxtop;
	r = 0;

	for (i = digit, cp = tmp + digit; i < maxtop; i++, cp++) {

		amin = (i >= bplus1) ? i-btop : 0;
		bmin = (i >= bplus1) ? btop : i;
		amax = (i > atop) ? atop : i;

		ap = a->dig + amin;
		bp = b->dig + bmin;

		*cp = 0;

		for (ai = amin; ai <= amax; ai++)
			r += (_mpl_long_t)(*ap++) * (_mpl_long_t)(*bp--);

		*cp = r & MPL_INT_MASK;
		r >>= MPL_INT_BITS;
	}

	*cp = r;

	ap = tmp + digit;
	bp = c->dig;

	/* Zero lower digits of the result. */
	for (i = 0; i < digit; i++)
		*bp++ = 0;

	for (; i <= maxtop; i++)
		*bp++ = *ap++;
	
	/* Zero unused upper digits of the result. */
	for (; i <= oldtop; i++)
		*bp++ = 0;

	mpl_canonicalize(c);

	return MPL_OK;
}

int
_mpl_mul_comba_ndig(mpl_int *c, const mpl_int *a, const mpl_int *b, int ndig)
{
	_mpl_int_t tmp[MPL_COMBA_STACK];
	_mpl_int_t *ap, *bp, *cp;
	_mpl_long_t r;
	int i, ai, rc, bplus1;
	unsigned int amin, bmin, amax;
	int atop, btop, oldtop;
	int n, nmax;

	atop = a->top;
	btop = b->top;
	oldtop = c->top;
	bplus1 = btop + 1;

	nmax = atop + btop + 2;
	ndig = MIN(ndig, nmax);
	n = (ndig == nmax) ? ndig-1 : ndig;

	if ((rc = mpl_ensure(c, n)) != MPL_OK)
		return rc;
	r = 0;

	for (i = 0, cp = tmp; i < n; i++, cp++) {

		amin = (i >= bplus1) ? i-btop : 0;
		bmin = (i >= bplus1) ? btop : i;
		amax = (i > atop) ? atop : i;

		ap = a->dig + amin;
		bp = b->dig + bmin;

		*cp = 0;

		for (ai = amin; ai <= amax; ai++)
			r += (_mpl_long_t)(*ap++) * (_mpl_long_t)(*bp--);

		*cp = r & MPL_INT_MASK;
		r >>= MPL_INT_BITS;
	}

	*cp = r;

	ap = tmp;
	bp = c->dig;
	c->top = ndig - 1;

	for (i = 0; i < ndig; i++)
		*bp++ = *ap++;

	for (; i <= oldtop; i++)
		*bp++ = 0;

	mpl_canonicalize(c);

	return MPL_OK;
}

/* Multiplies c = |a| * |b|, |a| > |b| using school O(n^2) multiplication. */
int
_mpl_mul_school(mpl_int *c, const mpl_int *a, const mpl_int *b)
{
	mpl_int tmp;
	_mpl_int_t *ap, *bp, *cp;
	_mpl_long_t r;
	int i, j, rc;
	int atop, btop;
	int maxtop;

	maxtop = a->top + b->top + 1;

	if ((rc = mpl_init(&tmp)) != MPL_OK)
		return rc;

	if ((rc = mpl_ensure(&tmp, maxtop+1)) != MPL_OK)
		return rc;

	atop = a->top;
	btop = b->top;
	tmp.top = maxtop;

	for (bp = b->dig, i = 0; i <= btop; ++bp, i++) {

		/* Reset carry and initialize pointers to c[i] and a. */
		r  = 0;
		cp = tmp.dig + i;
		ap = a->dig;

		/* Multiply-add with carry loop. Using index notation the following
		 * code implements c[i+j] += a[j]*b[i]. */
		for (j = 0; j <= atop; j++) {
			r += ((_mpl_long_t)*bp) * ((_mpl_long_t)(*ap++)) + ((_mpl_long_t)*cp);
			*cp++ = r & (_mpl_long_t)MPL_INT_MASK;
			r >>= (_mpl_long_t)MPL_INT_BITS;
		}

		*cp = r;
	}

	if ((rc = mpl_copy(c, &tmp)) != MPL_OK) {
		mpl_clear(&tmp);
		return rc;
	}

	mpl_clear(&tmp);
	mpl_canonicalize(c);

	return MPL_OK;
}

int
mpl_mul_dig(mpl_int *c, const mpl_int *a, _mpl_int_t b)
{
	int i, n, rc, sign;
	_mpl_long_t r;
	_mpl_int_t *ap, *cp;

	if (mpl_iszero(a) || b == 0) {
		mpl_zero(c);
		return MPL_OK;
	}

	if ((rc = mpl_ensure(c, a->top + 2)) != MPL_OK)
		return rc;

	n = a->top;
	ap = a->dig;
	sign = a->sign;

	/* Zero unused digits in result. */
	cp = c->dig + c->top;
	for (i = c->top; i > n; i--)
		*cp-- = 0;

	r = 0;
	cp = c->dig;

	for (i = 0; i <= n; i++) {
		r += (_mpl_long_t)b * (_mpl_long_t)(*ap++);
		*cp++ = r & (_mpl_long_t)MPL_INT_MASK;
		r >>= MPL_INT_BITS;
	}

	*cp = r;

	c->top = n + 1;
	c->sign = sign;

	mpl_canonicalize(c);

	return MPL_OK;
}

int
mpl_mul(mpl_int *c, const mpl_int *a, const mpl_int *b)
{
	int rc, sign;
	const mpl_int *u, *v;

	if (mpl_iszero(a) || mpl_iszero(b)) {
		mpl_zero(c);
		return MPL_OK;
	}

	sign = (a->sign != b->sign) ? MPL_SIGN_NEG : MPL_SIGN_POS;

	if (a->top > b->top) {
		u = a;
		v = b;
	} else {
		u = b;
		v = a;
	}

	if (MIN(u->top+1, v->top+1) >= MPL_KARATSUBA_CUTOFF)
		rc = _mpl_mul_karatsuba(c, u, v);
	else if (u->top + v->top + 2 <= MPL_COMBA_STACK &&
	    v->top+1 <= MPL_COMBA_DEPTH)
		rc = _mpl_mul_comba(c, u, v);
	else
		rc = _mpl_mul_school(c, u, v);

	if (rc != MPL_OK)
		return rc;

	c->sign = sign;
	return MPL_OK;
}

int
mpl_mul_ndig(mpl_int *c, const mpl_int *a, const mpl_int *b, int ndig)
{
	mpl_int tmp;
	_mpl_int_t *ap, *bp, *cp;
	_mpl_long_t r;
	int i, j, n, m, rc;
	int nmax, sign;

	if (mpl_iszero(a) || mpl_iszero(b)) {
		mpl_zero(c);
		return MPL_OK;
	}

	if (a->sign != b->sign)
		sign = MPL_SIGN_NEG;
	else
		sign = MPL_SIGN_POS;

	/* Clamp maximum number of result digits produced. */
	nmax = a->top + b->top + 2;
	ndig = MIN(nmax, ndig);

	if ((ndig <= MPL_COMBA_DEPTH || MIN(a->top+1, b->top+1) <= MPL_COMBA_DEPTH) &&
	    ndig <= MPL_COMBA_STACK) {
		if ((rc = _mpl_mul_comba_ndig(c, a, b, ndig)) != MPL_OK)
			return rc;
		c->sign = sign;
		return MPL_OK;
	}

	if ((rc = mpl_init(&tmp)) != MPL_OK)
		return rc;

	/* Ensure place for final carry digit. */
	if ((rc = mpl_ensure(&tmp, (ndig > nmax) ? ndig+1 : ndig)) != MPL_OK)
		return rc;

	n = MIN(ndig, b->top+1);

	for (bp = b->dig, i = 0; i < n; ++bp, i++) {

		/* Reset carry and initialize pointers to c[i] and a. */
		r  = 0;
		cp = tmp.dig + i;
		ap = a->dig;

		/* Each row is less by i if `a' has more digits than required. */
		m = MIN(ndig - i, a->top+1);

		/* Multiply-add with carry loop. Using index notation the following
		 * code implements c[i+j] += a[j]*b[i]. */
		for (j = 0; j < m; j++) {
			r += ((_mpl_long_t)*bp) * ((_mpl_long_t)(*ap++)) + ((_mpl_long_t)*cp);
			*cp++ = r & (_mpl_long_t)MPL_INT_MASK;
			r >>= (_mpl_long_t)MPL_INT_BITS;
		}

		/* 
		 * We always have place for the final carry digit and checking
		 * that i+j < ndig is worthless here.
		 */
		*cp = r;
	}

	tmp.top = ndig - 1;

	if ((rc = mpl_copy(c, &tmp)) != MPL_OK) {
		mpl_clear(&tmp);
		return rc;
	}

	c->sign = sign;

	mpl_clear(&tmp);
	mpl_canonicalize(c);

	return MPL_OK;
}

