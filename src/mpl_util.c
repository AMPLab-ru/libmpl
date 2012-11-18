#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mpl.h"
#include "mpl_common.h"

int
mpl_iszero(const mpl_int *a)
{
	return (a->top < 0 ? 1 : 0);
}

int
mpl_isone(const mpl_int *a)
{
	return (a->top == 0 && a->dig[0] == 1);
}

int
mpl_isneg(const mpl_int *a)
{
	return (a->sign == MPL_SIGN_NEG);
}

/* memset() for MP integer. */
void
mpl_zero(mpl_int *a)
{
	int i, top;

	top = a->top;

	for (i = 0; i <= top; i++)
		a->dig[i] = 0;

	a->sign = MPL_SIGN_POS;
	a->top = -1;
}

/* Removes excessive zeros and fixes the sign if needed. */
void
mpl_canonicalize(mpl_int *a)
{
	int top;

	top = a->top;

	while (top >= 0 && a->dig[top] == 0)
		--top;

	if (top < 0)
		a->sign = MPL_SIGN_POS;

	a->top = top;
}

/* Swaps two MP integers. */
int
mpl_swap(mpl_int *a, mpl_int *b)
{
	mpl_int tmp;

	if (a == b)
		return MPL_OK;

	memcpy(&tmp, a, sizeof(tmp));
	memcpy(a, b, sizeof(*a));
	memcpy(b, &tmp, sizeof(*b));

	return MPL_OK;
}

int
mpl_nr_bits(const mpl_int *a)
{
	_mpl_int_t dig;
	int res, cnt;

	if (mpl_iszero(a))
		return 0;

	res = a->top * MPL_INT_BITS;
	dig = a->dig[a->top];

	cnt = 0;
	while (dig > 0) {
		dig >>= 1;
		++cnt;
	}

	res += cnt;

	return res;
}

void
mpl_dbg(const mpl_int *a, FILE *fp)
{
	_mpl_int_t *ap;
	int i, top;

	fprintf(fp, "%p: top=%04d alloc=%04d: ", a, a->top, a->alloc);

	if (mpl_iszero(a)) {
		fprintf(fp, "0\n");
		return;
	}

	top = a->top;
	ap = a->dig;

	if (a->sign == MPL_SIGN_NEG)
		fprintf(fp, "-");
	else
		fprintf(fp, " ");

	for (i = top; i >= 0; i--)
		fprintf(fp, "%07X", ap[i]);
	fprintf(fp, "\n");
}

