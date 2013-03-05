/*
 * Multiple Precision Library.
 *
 * Grisha Sitkarev, 2011 (c)
 * <sitkarev@unixkomi.ru>
 *
 * Kostya Nikulov, 2012
 * <knikulov@yandex.com>
 *
 */

#ifndef MPL_H_
#define MPL_H_

#ifdef __cplusplus
extern "C" {
#endif

enum {
	MPL_OK	= 0,
	MPL_ERR,
	MPL_NOMEM,
	MPL_COMPOSITE
};

#define MPL_CMP_EQ		0
#define MPL_CMP_GT		1
#define MPL_CMP_LT		-1

#define MPL_SIGN_POS		0
#define MPL_SIGN_NEG		1

#define MPL_INT_BASE		268435456
#define MPL_INT_MASK		0x0FFFFFFF
#define MPL_INT_BITS		28
#define MPL_INT_BITS_ALL	32

typedef uint32_t _mpl_int_t;
typedef uint64_t _mpl_long_t;

/* Multiple precision integer structure. */
typedef struct mpl_int {
	unsigned int	flags;
	unsigned int	alloc;	/* number of allocated digits */
	int		sign;	/* MPL_SIGN_POS or MPL_SIGN_NEG */
	int		top;	/* index to top digit */
	_mpl_int_t	*dig;	/* malloc'ed array of digits */
} mpl_int;

void mpl_dbg(const mpl_int *a, FILE *fp);

/* Initializes MPL integer. */
int mpl_init(mpl_int *a);

/* Releases all memory associated with MPL integer. */
void mpl_clear(mpl_int *a);

/* Variable NULL-terminated list version of two routines above. */
int mpl_initv(mpl_int *a, ...);
void mpl_clearv(mpl_int *a, ...);

void mpl_zero(mpl_int *a);

int mpl_iszero(const mpl_int *a);
int mpl_isone(const mpl_int *a);
int mpl_iseven(const mpl_int *a);
int mpl_isodd(const mpl_int *a);
int mpl_isneg(const mpl_int *a);

void mpl_set_one(mpl_int *a);
void mpl_set_sint(mpl_int *a, int value);
void mpl_set_uint(mpl_int *a, unsigned int value);
int  mpl_set_uchar(mpl_int *a, const unsigned char *buf, int len);
int  mpl_set_str(mpl_int *a, const char *str, int base);

int mpl_to_uchar(const mpl_int *a, unsigned char *buf, int len);
int mpl_to_str(const mpl_int *a, char *str, int base, int len);

long int mpl_nr_bits(const mpl_int *a);

int mpl_shr(mpl_int *a, unsigned int nr);
int mpl_shl(mpl_int *a, unsigned int nr);

int mpl_add(mpl_int *c, const mpl_int *a, const mpl_int *b);
int mpl_sub(mpl_int *c, const mpl_int *a, const mpl_int *b);
int mpl_mul(mpl_int *c, const mpl_int *a, const mpl_int *b);
int mpl_mul_ndig(mpl_int *c, const mpl_int *a, const mpl_int *b, int ndig);
int mpl_mul_dig(mpl_int *c, const mpl_int *a, _mpl_int_t b);
int mpl_div(mpl_int *q, mpl_int *r, const mpl_int *y, const mpl_int *x);
int mpl_sqr(mpl_int *c, const mpl_int *x);

int mpl_gcd(mpl_int *c, const mpl_int *a, const mpl_int *b);

int mpl_random(mpl_int *a, int size, int (*rnd)(void *buf, size_t len, void *rndctx), void *rndctx);

int mpl_primality_miller_rabin(const mpl_int *a, int r, int (*rnd)(void *buf, size_t len, void *rndctx), void *rndctx);

int mpl_reduce_barrett_setup(mpl_int *mu, const mpl_int *b);
int mpl_reduce_barrett(mpl_int *c, const mpl_int *a, const mpl_int *b, const mpl_int *mu);

int mpl_mod_inv(mpl_int *c, const mpl_int *a, const mpl_int *b);
int mpl_mod_exp(mpl_int *c, const mpl_int *a, const mpl_int *y, const mpl_int *b);

int mpl_cmp(const mpl_int *a, const mpl_int *b);
int mpl_abs_cmp(const mpl_int *a, const mpl_int *b);

int mpl_copy(mpl_int *dst, const mpl_int *src);
int mpl_swap(mpl_int *a, mpl_int *b);

int mpl_and(mpl_int *c, const mpl_int *a, const mpl_int *b);
int mpl_or(mpl_int *c, const mpl_int *a, const mpl_int *b);
int mpl_xor(mpl_int *c, const mpl_int *a, const mpl_int *b);

int mpl_check_bit(const mpl_int *a, long int n);

#ifdef __cplusplus
}
#endif

#endif

