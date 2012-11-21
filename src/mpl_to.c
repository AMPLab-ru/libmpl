#include <stdio.h>
#include <limits.h>
#include <stdint.h>

#include "mpl.h"

int
mpl_to_str(char *buf, int len, int base, const mpl_int *a)
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

	/* estimate the lenght of buf */
	if (len < mpl_nr_bits(a)/nb + 3)
		return MPL_ERR;

	if (mpl_iszero(a)) {
		*buf++ =  '0';
		*buf   = '\0';
		return MPL_OK;
	}

	if ((rc = mpl_initv(&x, &y, &r, NULL)) != MPL_OK)
		return rc;

	if ((rc = mpl_copy(&y, a)) != MPL_OK)
		goto err;

	mpl_set_uint(&x, base);

	if (mpl_isneg(&y)) {
		*buf++ = '-';
		y.sign = MPL_SIGN_POS;
	}

	sp = buf;
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
	sp = buf;

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

