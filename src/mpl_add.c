#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mpl.h"
#include "mpl_common.h"

extern int _mpl_sub(mpl_int *c, const mpl_int *a, const mpl_int *b);

extern int mpl_canonicalize(mpl_int *a);

/* Adds c = |a| + |b|, c can point to a or b. */
int
_mpl_add(mpl_int *c, const mpl_int *a, const mpl_int *b)
{
	_mpl_int_t *ap, *bp, *cp, cr;
	int btop, ctop, i, oldtop, rc;

	if (a->top < b->top) {
		const mpl_int *tmp;
		tmp = a;
		a = b;
		b = tmp;
	}

	/* Ensure we have space for an additional digit. */
	if ((rc = mpl_ensure(c, a->top+2)) != MPL_OK)
		return rc;

	ap = a->dig;
	bp = b->dig;
	cp = c->dig;
	btop = b->top;
	ctop = a->top + 1;
	oldtop = c->top;

	cr = 0;

	for (i = 0; i <= btop; i++) {
		*cp = (*ap++) + (*bp++) + cr;
		cr = *cp >> MPL_INT_BITS;
		*cp++ &= MPL_INT_MASK;
	}

	for (i = btop+1; i < ctop; i++) {
		*cp = (*ap++) + cr;
		cr = *cp >> MPL_INT_BITS;
		*cp++ &= MPL_INT_MASK;
	}

	*cp++ = cr;

	/* Zero unused digits in the result. */
	for (i = ctop+1; i <= oldtop; i++)
		*cp++ = 0;

	c->top = ctop;

	mpl_canonicalize(c);

	return MPL_OK;
}

int
mpl_add(mpl_int *c, const mpl_int *a, const mpl_int *b)
{
	int rc;

	if (a->sign == b->sign) {
		/* Do addition c = |a| + |b|. */
		if ((rc = _mpl_add(c, a, b)) != MPL_OK)
			return rc;
		/* Result sign is the sign of a or b. */
		c->sign = a->sign;
	} else {
		if (mpl_abs_cmp(a, b) == MPL_CMP_GT) {
			if ((rc = _mpl_sub(c, a, b)) != MPL_OK)
				return rc;
			c->sign = a->sign;
		} else {
			if ((rc = _mpl_sub(c, b, a)) != MPL_OK)
				return rc;
			c->sign = b->sign;
		}
	}

	return MPL_OK;
}

