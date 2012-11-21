#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "mpl.h"
#include "mpl_common.h"

void
mpl_set_sint(mpl_int *a, int val)
{
	int i, oldtop;
	_mpl_int_t *dp;

	oldtop = a->top;

	a->top = 0;
	a->sign = (val >= 0 ? MPL_SIGN_POS : MPL_SIGN_NEG);

	if (val >= 0)
		a->dig[0] = val & MPL_INT_MASK;
	else
		a->dig[0] = -val & MPL_INT_MASK;

	dp = a->dig + 1;

	for (i = 1; i <= oldtop; i++)
		*dp++ = 0;
}

void
mpl_set_uint(mpl_int *a, unsigned int val)
{
	int i, oldtop;
	_mpl_int_t *dp;

	oldtop = a->top;

	a->top = 0;
	a->sign = MPL_SIGN_POS;
	a->dig[0] = val & MPL_INT_MASK;

	dp = a->dig + 1;

	for (i = 1; i <= oldtop; i++)
		*dp++ = 0;
}

int
mpl_set(mpl_int *a, unsigned long *val, unsigned int nval, int sign)
{
	_mpl_int_t *ap;
	int i, rc, top;

	if ((rc = mpl_ensure(a, nval)) != MPL_OK)
		return rc;

	ap = a->dig;
	top = a->top;

	/* Copy digits to MP integer. */
	for (i = 0; i < nval; i++)
		*ap++ = *val++ & MPL_INT_MASK;

	/* All digits beyond top (if any) are zeroed. */
	if (top >= nval) {
		for (i = nval; i <= top; i++)
			*ap++ = 0;
	}

	a->top = nval-1;
	a->sign = sign;

	return MPL_OK;
}

void
mpl_set_one(mpl_int *a)
{
	mpl_set_uint(a, 1);
}

int
mpl_set_str(mpl_int *a, const char *str, int base)
{
	mpl_int tmp;
	int i, n, rc;
	int dig, sign;
	char c;

	if (base < 2 || base > 36)
		return MPL_ERR;

	rc = mpl_init(&tmp);
	if (rc != MPL_OK)
		return rc;

	if (*str == '-') {
		sign = MPL_SIGN_NEG;
		str++;
	} else {
		sign = MPL_SIGN_POS;
	}

	mpl_zero(a);

	n = strlen(str);
	for (i = 0; i < n; i++) {

		c = *str++;

		if (c >= '0' && c <= '9')
			dig = c - '0';
		else
			dig = toupper(c) - 'A' + 10;

		if (dig < 0 || dig >= base) {
			rc = MPL_ERR;
			goto err;
		}

		rc = mpl_mul_dig(a, a, base);
		if (rc != MPL_OK)
			goto err;

		mpl_set_uint(&tmp, dig);
		
		rc = mpl_add(a, a, &tmp);
		if (rc != MPL_OK)
			goto err;
	}

	a->sign = sign;
	rc = MPL_OK;
err:
	mpl_clear(&tmp);
	return rc;
}

