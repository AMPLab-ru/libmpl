#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mpl.h"
#include "mpl_common.h"

extern int _mpl_add(mpl_int *c, const mpl_int *a, const mpl_int *b);

extern void mpl_canonicalize(mpl_int *a);

/* Subtracts c = |a| - |b|. Algorithm requires that |a| > |b|. */
int
_mpl_sub(mpl_int *c, const mpl_int *a, const mpl_int *b)
{
	_mpl_int_t *ap, *bp, *cp, cr;
	int btop, ctop, oldtop;
	int i, rc;

	if ((rc = mpl_ensure(c, a->top+1)) != MPL_OK)
		return rc;

	btop = b->top;
	ctop = a->top;
	oldtop = c->top;
	ap = a->dig;
	bp = b->dig;
	cp = c->dig;
	cr = 0;

	for (i = 0; i <= btop; i++) {
		*cp = (*ap++) - (*bp++) - cr;
		cr = *cp >> (MPL_INT_BITS_ALL-1);
		*cp++ &= MPL_INT_MASK;
	}

	for (i = btop+1; i <= ctop; i++) {
		*cp = (*ap++) - cr;
		cr = *cp >> (MPL_INT_BITS_ALL-1);
		*cp++ &= MPL_INT_MASK;
	}

	/* Zero unused digits in the result. */
	for (i = ctop+1; i <= oldtop; i++)
		*cp++ = 0;

	c->top = ctop;

	mpl_canonicalize(c);

	return MPL_OK;
}

int
mpl_sub(mpl_int *c, const mpl_int *a, const mpl_int *b)
{
	int rc;

	if (a->sign == b->sign) {
		switch (mpl_abs_cmp(a, b)) {
		case MPL_CMP_GT:
			/* |a| > |b|, c = sign(a)(|a| - |b|) */
			if ((rc = _mpl_sub(c, a, b)) != MPL_OK)
				return rc;
			c->sign = a->sign;
			break;
		case MPL_CMP_LT:
			/* |a| < |b|, c = inv(sign(b))(|b| - |a|) */
			if ((rc = _mpl_sub(c, b, a)) != MPL_OK)
				return rc;
			c->sign = (b->sign == MPL_SIGN_POS ? MPL_SIGN_NEG : MPL_SIGN_POS);
			break;
		case MPL_CMP_EQ:
			/* |a| = |b|, c = 0 */
			mpl_zero(c);
			break;
		}
	} else {
		/* c = sign(a)(|a| + |b|). */
		if ((rc = _mpl_add(c, a, b)) != MPL_OK)
			return rc;
		c->sign = a->sign;
	}

	return MPL_OK;
}

