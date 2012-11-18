#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "mpl.h"
#include "mpl_common.h"

int
mpl_init(mpl_int *p)
{
	unsigned int allocsz;
	int i;

	assert(p != NULL);

	allocsz = MPL_ALLOC_DEFAULT + MPL_ALLOC_APPEND;

	p->dig = malloc(allocsz * sizeof(_mpl_int_t));

	if (p->dig == NULL)
		return MPL_NOMEM;

	p->flags = 0;
	p->sign = MPL_SIGN_POS;
	p->alloc = allocsz;
	p->top = -1;

	for (i = 0; i < allocsz; i++)
		p->dig[i] = 0;
	
	return MPL_OK;
}

int
mpl_initv(mpl_int *a, ...)
{
	unsigned int allocsz, cnt;
	va_list ap, ap_copy;
	_mpl_int_t *dp;
	mpl_int *p;

	allocsz = MPL_ALLOC_DEFAULT + MPL_ALLOC_APPEND;

	cnt = 0;
	p = a;
	va_start(ap, a);
	va_copy(ap_copy, ap);

	while (p != NULL) {
		int i;

		dp = malloc(allocsz * sizeof(_mpl_int_t));
		if (dp == NULL)
			goto nomem;
		++cnt;
		p->dig = dp;
		p->flags = 0;
		p->sign = MPL_SIGN_POS;
		p->alloc = allocsz;
		p->top = -1;

		for (i = 0; i < allocsz; i++)
			p->dig[i] = 0;

		p = va_arg(ap, mpl_int *);
	}

	va_end(ap_copy);
	va_end(ap);

	return MPL_OK;
nomem:
	p = a;
	while (p != NULL && cnt > 0) {
		--cnt;
		free(p->dig);
		p = va_arg(ap_copy, mpl_int *);
	}
	va_end(ap_copy);
	va_end(ap);

	return MPL_NOMEM;
}

