#ifndef MPL_COMMON_H_
#define MPL_COMMON_H_

#define MPL_ALLOC_CHUNK		64
#define MPL_ALLOC_DEFAULT	MPL_ALLOC_CHUNK
#define MPL_ALLOC_APPEND		8

#define MPL_KARATSUBA_CUTOFF	80

#define MPL_COMBA_DEPTH		256
#define MPL_COMBA_STACK		1024

#define MIN(a, b)		((a) > (b) ? (b) : (a))
#define MAX(a, b)		((a) > (b) ? (a) : (b))

#ifndef NDEBUG
#define MPL_DEBUG(fmt, arg...)	{ fprintf(stderr, "MPDBG: " fmt "\n", ##arg); fflush(stderr); }
#else
#define MPL_DEBUG(fmt, arg...)
#endif

int mpl_ensure(mpl_int *p, unsigned int new_size);

#endif

