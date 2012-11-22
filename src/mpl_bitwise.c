#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "mpl.h"
#include "mpl_common.h"

#define _MPL_AND	0
#define _MPL_OR		1
#define _MPL_XOR	2

extern void mpl_canonicalize(mpl_int *a);

static int
_mpl_bitwise(mpl_int *c, const mpl_int *a, const mpl_int *b, int op)
{
	_mpl_int_t *ap, *bp, *cp;
	int atop, btop;
	int rc, i;

	if (a->top < b->top) {
		const mpl_int *tmp;

		tmp = a;
		a = b;
		b = tmp;
	}

	rc = mpl_ensure(c, a->top);
	if (rc != MPL_OK)
		return rc;

	ap = a->dig;
	bp = b->dig;
	cp = c->dig;
	atop = a->top;
	btop = b->top;

	c->top = atop;
	switch (op) {
	case _MPL_AND:
		for (i = 0; i <= btop; i++)
			*cp++ = (*ap++) & (*bp++);

		for (i = btop+1; i < atop; i++)	
			*cp++ = 0;

		break;
	case _MPL_OR:
		for (i = 0; i <= btop; i++)
			*cp++ = (*ap++) | (*bp++);

		for (i = btop+1; i < atop; i++)	
			*cp++ = *ap++;

		break;
	case _MPL_XOR:
		for (i = 0; i <= btop; i++)
			*cp++ = (*ap++) ^ (*bp++);

		for (i = btop+1; i < atop; i++)	
			*cp++ = *ap++;

		break;
	}

	mpl_canonicalize(c);

	return MPL_OK;
}

int
mpl_and(mpl_int *c, const mpl_int *a, const mpl_int *b)
{
	return _mpl_bitwise(c, a, b, _MPL_AND);
}

int
mpl_or(mpl_int *c, const mpl_int *a, const mpl_int *b)
{
	return _mpl_bitwise(c, a, b, _MPL_OR);
}

int
mpl_xor(mpl_int *c, const mpl_int *a, const mpl_int *b)
{
	return _mpl_bitwise(c, a, b, _MPL_XOR);
}

