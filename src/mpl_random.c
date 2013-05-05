#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>

#include "mpl.h"
#include "mpl_common.h"

int
_mpl_random_bits(mpl_int *a, long nbits,
	  int (*rnd)(void *buf, size_t size, void *rndctx),
	  void *rndctx)
{
	/* nc - n bits in c */
	unsigned char buf[1024], *bufptr;
	_mpl_int_t *dig;
	int i, nb, nc, rc, ns;
	int nbuf, ndig, oldtop;
	int size;
	unsigned char c;

	ndig = nbits / MPL_INT_BITS;
	ndig += nbits % MPL_INT_BITS ? 1 : 0;

	size = nbits / CHAR_BIT;
	size += nbits % CHAR_BIT ? 1 : 0;

	if ((rc = mpl_ensure(a, ndig)) != MPL_OK)
		return rc;

	oldtop = a->top;
	a->top = ndig-1;
	a->sign = MPL_SIGN_POS;
	dig = a->dig;

	c = ns = nb = nc = nbuf = 0;
	bufptr = NULL;
	rc = MPL_ERR;

	while (nbits > 0) {

		nb = (nbits >= MPL_INT_BITS) ? MPL_INT_BITS : nbits;
		ns = nb;

		*dig = 0;
			
		/* current mpl word */
		while (nb > 0) {

			if (nc <= 0) {
				/* get next char into c */

				if (nbuf <= 0) {
					/* get entropy to buffer */
					int nread, res;

					nread = (size >= sizeof(buf)) ? sizeof(buf) : size;

					res = (*rnd)(buf, nread, rndctx);

					if (res == -1)
						goto out;

					nbuf = nread;
					bufptr = buf;
				}

				c = *bufptr++;

			
				nc = CHAR_BIT;
				--nbuf;
			}

			if (nb >= nc) {
				*dig |= c << (ns - nb);
				nb -= nc;
				nbits -= nc;
				nc = 0;
				--size;
			} else {
				*dig |= (c & ((1 << nb)-1)) << (ns - nb);
				nc -= nb;
				nbits -= nb;
				c >>= nb;
				nb = 0;
			}
		}

		++dig;
	}

	for (i = ndig; i <= oldtop; i++)
		*dig++ = 0;

	mpl_canonicalize(a);

	rc = MPL_OK;
out:
	return rc;
}

int
mpl_random(mpl_int *a, int size,
	  int (*rnd)(void *buf, size_t size, void *rndctx),
	  void *rndctx)
{
	int rc;
	int offset, ndig;
	_mpl_int_t *dig;
	unsigned char c;


	ndig = (size * CHAR_BIT) / MPL_INT_BITS;
	ndig += (size * CHAR_BIT) % MPL_INT_BITS ? 1: 0;
	
	if ((rc = _mpl_random_bits(a, size * CHAR_BIT, rnd, rndctx)) != MPL_OK)
		return rc;

	if (size != 0) {
		/*
		 * Last random byte is required to have two top
		 * bits set. This guarantees that the
		 * multiplication of two such values will always
		 * produce carry to the top bit of the result.
		 */
		/* shift to start of last byte */
		dig = a->dig + ((size - 1) * CHAR_BIT) / MPL_INT_BITS;

		offset = ((size - 1) * CHAR_BIT) % MPL_INT_BITS;
		c = 0xc0;

		*dig |= (MPL_INT_MASK & (c << offset));
		if (offset > MPL_INT_BITS - CHAR_BIT) {
			dig++;
			*dig |= (c >> (MPL_INT_BITS - offset));
		}

		a->top = ndig - 1;
	}

	mpl_canonicalize(a);

	return MPL_OK;
}

/* return random number b from range [0; a) */
int
mpl_rand_below(mpl_int *b, mpl_int *a, 
	  int (*rnd)(void *buf, size_t size, void *rndctx),
	  void *rndctx)
{
	long size;
	int rc;

	size = mpl_nr_bits(a);

	do {
		if ((rc = _mpl_random_bits(b, size, rnd, rndctx)) != MPL_OK)
			goto err;
	} while (mpl_cmp(a, b) <= 0);

	return MPL_OK;
err:
	return rc;
}

