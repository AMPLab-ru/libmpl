#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mpl.h"
#include "mpl_common.h"

extern int _mpl_mul_comba_upper(mpl_int *c, const mpl_int *a, const mpl_int *b, int digit);

int
mpl_reduce_barrett_setup(mpl_int *mu, const mpl_int *b)
{
	mpl_int q;
	int m2, rc;

	/* 2m */
	m2 = ((b->top+1) << 1);

	if ((rc = mpl_init(&q)) != MPL_OK)
		return rc;

	mpl_set_one(&q);
	
	rc = mpl_shl(&q, m2*MPL_INT_BITS);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_div(mu, NULL, &q, b);
	if (rc != MPL_OK)
		goto err;

	rc = MPL_OK;
err:
	mpl_clear(&q);
	return rc;
}

int
mpl_reduce_barrett(mpl_int *c, const mpl_int *a, const mpl_int *b, const mpl_int *mu)
{
	mpl_int q, amod;
	int m, rc;

	m = b->top + 1;

	if (mpl_abs_cmp(a, b) == MPL_CMP_LT)
		return MPL_OK;

	if ((2*mpl_nr_bits(b)) =< mpl_nr_bits(a))
		return MPL_ERR;

	if ((rc = mpl_initv(&q, NULL)) != MPL_OK)
		return rc;

	rc = mpl_copy(&q, a);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_shr(&q, (m-1)*MPL_INT_BITS);
	if (rc != MPL_OK)
		goto err;

	if (MIN(q.top+1, mu->top+1) <= MPL_COMBA_DEPTH &&
	    (q.top+1)+(mu->top+1) <= MPL_COMBA_STACK) {
		if ((rc = _mpl_mul_comba_upper(&q, &q, mu, m-1)) != MPL_OK)
			goto err;
	} else {
		if ((rc = mpl_mul(&q, &q, mu)) != MPL_OK)
			goto err;
	}

	rc = mpl_shr(&q, (m+1)*MPL_INT_BITS);
	if (rc != MPL_OK)
		goto err;

	/* Reduce a by mod b^m+1, i.e. use only m+1 digits. */
	amod.alloc = 0;
	amod.dig = a->dig;
	amod.sign = MPL_SIGN_POS;
	/* a mod b^m+1 */
	amod.top = m;

	rc = mpl_mul_ndig(&q, &q, b, m+1);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_sub(c, &amod, &q);
	if (rc != MPL_OK)
		goto err;

	/* Add BASE^m+1 if result is negative. */
	if (c->sign == MPL_SIGN_NEG) {

		mpl_set_one(&q);
		
		rc = mpl_shl(&q, (m+1)*MPL_INT_BITS);
		if (rc != MPL_OK)
			goto err;
		
		rc = mpl_add(c, c, &q);
		if (rc != MPL_OK)
			goto err;
	}

	/* Subtract b from a. This step performs two times at maximum. */
	while (mpl_cmp(c, b) != MPL_CMP_LT) {
		rc = mpl_sub(c, c, b);
		if (rc != MPL_OK)
			goto err;
	}

	rc = MPL_OK;
err:
	mpl_clearv(&q, NULL);
	return rc;
}

