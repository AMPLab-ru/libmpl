#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mpl.h"
#include "mpl_common.h"

int
mpl_copy(mpl_int *dst, const mpl_int *src)
{	
	_mpl_int_t *dp, *sp;
	int src_top, dst_top;
	int i, rc;

	if (dst == src)
		return MPL_OK;

	if (mpl_iszero(src)) {
		mpl_zero(dst);
		return MPL_OK;
	}

	if ((rc = mpl_ensure(dst, src->top+1)) != MPL_OK)
		return rc;

	src_top = src->top;
	dst_top = dst->top;
	dp = dst->dig;
	sp = src->dig;

	for (i = 0; i <= src_top; i++)
		*dp++ = *sp++;

	for (; i <= dst_top; i++)
		*dp++ = 0;

	dst->top = src_top;
	dst->sign = src->sign;

	return MPL_OK;
}

