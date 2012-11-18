#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mpl.h"
#include "mpl_common.h"

/* Compares absolute values |a| and |b|. */
int
_mpl_abs_cmp(const mpl_int *a, const mpl_int *b)
{
	_mpl_int_t *ap, *bp;
	int i;

	if (a->top > b->top)
		return MPL_CMP_GT;
	else if (a->top < b->top)
		return MPL_CMP_LT;

	/* a and b have equal no of digits */
	ap = a->dig + a->top;
	bp = b->dig + b->top;

	for (i = a->top; i >= 0; i--) {
		if (*ap != *bp) {
			if (*ap > *bp)
				return MPL_CMP_GT;
			else if (*ap < *bp)
				return MPL_CMP_LT;
		}
		--ap; --bp;
	}

	return MPL_CMP_EQ;
}

int
mpl_abs_cmp(const mpl_int *a, const mpl_int *b)
{	
	if (a == b || (mpl_iszero(a) && mpl_iszero(b)))
		return MPL_CMP_EQ;

	return _mpl_abs_cmp(a, b);
}

int
mpl_cmp(const mpl_int *a, const mpl_int *b)
{
	int res;

	if (a == b || (mpl_iszero(a) && mpl_iszero(b)))
		return MPL_CMP_EQ;

	if (a->sign != b->sign)
		return a->sign == MPL_SIGN_POS ? MPL_CMP_GT : MPL_CMP_LT;
 
	if (a->sign == MPL_SIGN_NEG)
		res = _mpl_abs_cmp(b, a);
	else
		res = _mpl_abs_cmp(a, b);

	return res;
}

