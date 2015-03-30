#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "mpl.h"
#include "mpl_common.h"

extern void mpl_canonicalize(mpl_int *a);

/* Simple binary exponentiation algorithm. */
int
mpl_exp(mpl_int *c, const mpl_int *a, const mpl_int *b)
{
	mpl_int res, tmp, u, v;
	int rc;

	rc = MPL_ERR;

	if ((rc = mpl_initv(&res, &tmp, &u, &v, NULL)) != MPL_OK)
		return rc;

	rc = mpl_copy(&u, a);
	if (rc != MPL_OK)
		goto err;

	rc = mpl_copy(&v, b);
	if (rc != MPL_OK)
		goto err;
	
	mpl_set_one(&res);

	while(!mpl_iszero(&v)) {
		if (mpl_isodd(&v)) {
			rc = mpl_mul(&res, &res, &u);
			if (rc != MPL_OK)
				goto err;
		}
		
		rc = mpl_shr(&v, 1);
		if (rc != MPL_OK)
			goto err;
		
		rc = mpl_mul(&u, &u, &u);
		if (rc != MPL_OK)
			goto err;
	}

	rc = mpl_copy(c, &res);
	if (rc != MPL_OK)
		goto err;

	mpl_canonicalize(c);

	rc = MPL_OK;
err:
	mpl_clearv(&res, &tmp, &u, &v, NULL);
	return rc;
}

