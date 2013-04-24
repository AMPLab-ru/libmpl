#include <stdio.h>
#include <limits.h>
#include <stdint.h>

#include "mpl.h"

int
mpl_to_str(const mpl_int *a, char *str, int base, int len)
{
	mpl_int x, y, r;
	int i, rc, nb, cmp;
	char *sp, *end;
	char c;

	if (base < 2 || base > 36)
		return MPL_ERR;

	nb = 0;
	for (i = base; i > 0; i >>= 1)
		nb++;

	/* estimate the lenght of str */
	if (len < mpl_nr_bits(a)/nb + 3)
		return MPL_ERR;

	if (mpl_iszero(a)) {
		*str++ =  '0';
		*str   = '\0';
		return MPL_OK;
	}

	if ((rc = mpl_initv(&x, &y, &r, NULL)) != MPL_OK)
		return rc;

	if ((rc = mpl_copy(&y, a)) != MPL_OK)
		goto err;

	mpl_set_uint(&x, base);

	if (mpl_isneg(&y)) {
		*str++ = '-';
		y.sign = MPL_SIGN_POS;
	}

	sp = str;
	cmp = mpl_cmp(&y, &x);
	while (cmp == MPL_CMP_GT || cmp == MPL_CMP_EQ) {
		rc = mpl_div(&y, &r, &y, &x);
		if (rc != MPL_OK)
			goto err;

		c = (char) r.dig[0];
		if (c >= 0 && c <= 9)
			*sp = c + '0';
		else
			*sp = c + 'a' - 10;

		sp++;
		cmp = mpl_cmp(&y, &x);
	}

	c = (char) y.dig[0];
	if (c > 0) {
		if (c <= 9)
			*sp = c + '0';
		else
			*sp = c + 'a' - 10;
		sp++;
	}
	*sp-- = '\0';

	end = sp;
	sp = str;

	while (sp < end) {
		char tmp;

		tmp    = *sp;
		*sp++  = *end;
		*end-- = tmp;
	}

	rc = MPL_OK;
err:
	mpl_clearv(&x, &y, &r, NULL);

	return rc;
}

int 
mpl_to_uint(mpl_int *a, unsigned long *val)
{
	int nbits;

	nbits = mpl_nr_bits(a);

	if (mpl_isneg(a) || nbits > sizeof(int) * CHAR_BIT)
		return MPL_ERR;

	*val = a->dig[0];
	if (nbits > MPL_INT_BITS) {
		*val <<= MPL_INT_BITS;
		*val += a->dig[1];
	}

	return MPL_OK;
}
