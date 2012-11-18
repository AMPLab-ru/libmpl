#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>

#include "mpl.h"
#include "mpl_common.h"

void
mpl_clear(mpl_int *p)
{
	assert(p != NULL);

	if (p->dig != NULL)
		free(p->dig);
}

void
mpl_clearv(mpl_int *a, ...)
{
	va_list ap;

	va_start(ap, a);

	while (a != NULL) {
		if (a->dig != NULL)
			free(a->dig);
		a = va_arg(ap, mpl_int *);
	}

	va_end(ap);
}

