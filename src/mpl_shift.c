#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mpl.h"
#include "mpl_common.h"

extern void mpl_canonicalize(mpl_int *a);

int
mpl_shr(mpl_int *a, unsigned int nr)
{
	_mpl_int_t *dst, *src;
	unsigned int nbits, ndigs;
	int i, n;

	if (nr == 0 || mpl_iszero(a))
		return MPL_OK;

	if (nr >= mpl_nr_bits(a)) {
		mpl_zero(a);
		return MPL_OK;
	}

	ndigs = nr / MPL_INT_BITS;
	nbits = nr % MPL_INT_BITS;

	n = a->top + 1 - ndigs;
	dst = a->dig;
	src = a->dig + ndigs;

	if (nbits > 0) {
		_mpl_int_t mask, shift;

		mask = (~(_mpl_int_t)0 & MPL_INT_MASK) >> (MPL_INT_BITS - nbits);
		shift = MPL_INT_BITS - nbits;

		for (i = 1; i < n; i++) {
			*dst = *src >> nbits;
			*dst |= (*(src+1) & mask) << shift;
			++dst; ++src;
		}

		*dst = *src >> nbits;

	} else {
		for (i = 0; i < n; i++)
			*dst++ = *src++;
	}

	for (i = a->top; i > (a->top - ndigs); i--)
		a->dig[i] = 0;

	a->top -= ndigs;

	mpl_canonicalize(a);

	return MPL_OK;
}

int
mpl_shl(mpl_int *a, unsigned int nr)
{
	_mpl_int_t *dst, *src;
	unsigned int ensure;
	unsigned int nbits, ndigs;
	int i, n, rc;

	if (nr == 0 || mpl_iszero(a))
		return MPL_OK;

	ndigs = nr / MPL_INT_BITS;
	nbits = nr % MPL_INT_BITS;
	ensure = a->top + 1 + ndigs;
	ensure += nbits > 0 ? 1 : 0;

	if ((rc = mpl_ensure(a, ensure)) != MPL_OK)
		return rc;

	n = a->top + 1;
	dst = a->dig + a->top + ndigs;
	src = a->dig + a->top;

	if (nbits > 0) {
		_mpl_int_t mask, shift;

		shift = MPL_INT_BITS - nbits;
		mask = (~(_mpl_int_t)0 << shift) & MPL_INT_MASK;

		for (i = 0; i < n; i++) {
			*(dst+1) |= (*src & mask) >> shift;
			*dst = (*src << nbits) & MPL_INT_MASK;
			--dst; --src;
		}

	} else {
		for (i = 0; i < n; i++)
			*dst-- = *src--;
	}

	for (i = 0; i < ndigs; i++)
		a->dig[i] = 0;

	a->top += ndigs + (nbits ? 1 : 0);

	mpl_canonicalize(a);

	return MPL_OK;
}

