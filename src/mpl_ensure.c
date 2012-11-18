#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mpl.h"
#include "mpl_common.h"

/* Ensures place for new_size digits in the MP integer, resizing if necessary. */
int
mpl_ensure(mpl_int *p, unsigned int new_size)
{
	void *tmp;
	unsigned int allocsz;
	int i;

	if (p->alloc >= new_size)
		return MPL_OK;

	/*
	 * Calculate the size rounding new_size up to MPL_ALLOC_CHUNK.
	 * Additional MPL_ALLOC_APPEND bytes are attached to the block
	 * to reduce overhead caused by small changes. 
	 */
	allocsz = (new_size / MPL_ALLOC_CHUNK) * MPL_ALLOC_CHUNK +
		  (new_size % MPL_ALLOC_CHUNK ? MPL_ALLOC_CHUNK : 0);
	allocsz += MPL_ALLOC_APPEND;

	tmp = realloc(p->dig, allocsz * sizeof(_mpl_int_t));

	if (tmp == NULL)
		return MPL_NOMEM;

	p->dig = tmp;
	for (i = p->alloc; i < allocsz; i++)
		p->dig[i] = 0;
	p->alloc = allocsz;

	return MPL_OK;
}

