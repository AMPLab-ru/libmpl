#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mpl.h"
#include "mpl_common.h"

int
mpl_gcd(mpl_int *c, const mpl_int *a, const mpl_int *b)
{
	mpl_int x, y, u, v;
	int rc, k;
	
	if ((rc = mpl_initv(&u, &v, NULL)) != MPL_OK)
		return rc;

	x.dig = a->dig;
	x.alloc = a->alloc;
	x.top = a->top;
	x.sign = MPL_SIGN_POS;

	y.dig = b->dig;
	y.alloc = b->alloc;
	y.top = b->top;
	y.sign = MPL_SIGN_POS;

	rc = mpl_copy(&u, &x);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_copy(&v, &y);
	if (rc != MPL_OK)
		goto err;

	k = 0;
	while (mpl_iseven(&v) && mpl_iseven(&u)) {
		rc = mpl_shr(&u, 1);
		if (rc != MPL_OK)
			goto err;
		rc = mpl_shr(&v, 1);
		if (rc != MPL_OK)
			goto err;
		++k;
	}

	do {
		/* Reduce u by factor of two. */
		while (mpl_iseven(&u)) {
			rc = mpl_shr(&u, 1);
			if (rc != MPL_OK)
				goto err;
		}

		/* Reduce v by factor of two. */
		while (mpl_iseven(&v)) {
			rc = mpl_shr(&v, 1);
			if (rc != MPL_OK)
				goto err;
		}

		/* Reduce greatest of u or v. */
		if (mpl_cmp(&u, &v) != MPL_CMP_LT) {
			/* u = (u - v) / 2 */
			rc = mpl_sub(&u, &u, &v);
			if (rc != MPL_OK)
				goto err;
			rc = mpl_shr(&u, 1);
			if (rc != MPL_OK)
				goto err;
		} else {
			/* v = (v - u) / 2 */
			rc = mpl_sub(&v, &v, &u);
			if (rc != MPL_OK)
				goto err;
			rc = mpl_shr(&v, 1);
			if (rc != MPL_OK)
				goto err;
		}

	} while (!mpl_iszero(&u) && !mpl_iszero(&v));

	if (mpl_iszero(&u)) {
		rc = mpl_copy(c, &v);
		if (rc != MPL_OK)
			goto err;
	} else {
		rc = mpl_copy(c, &u);
		if (rc != MPL_OK)
			goto err;
	}

	if (k > 0) {
		rc = mpl_shl(c, k);
		if (rc != MPL_OK)
			goto err;
	}

	rc = MPL_OK;
err:
	mpl_clearv(&u, &v, NULL);
	return rc;
}

