#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <limits.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "mpl.h"

#define	BUFSIZE	LINE_MAX
#define	STKSIZE	128
#define	ARGVMAX	2

#define	OK		0
#define NO_MEMORY	1
#define	WRONG_FORMAT	2
#define	WRONG_NARGS	3
#define	WRONG_STACK	4
#define	STACK_OVERFLOW	5
#define	WRONG_ARGUMENT	6
#define	IMPOSSIBLE	7
#define	UNKNOWN		8

#define	ARRAY_SIZE(a)	(sizeof(a)/sizeof(a[0]))

struct function {
	const char *name;
	int nargs;
	int nin;
	int nout;
	int (*func)(int argc, char **argv);
};

void stk_init(void);
void stk_grow(int n);
void stk_free(void);
int stk_nelems(void);
int stk_places(void);
int stk_add_place(void);
mpl_int *stk(int n);

int process_input(char *buf, int len);
int urandom(void *buf, size_t len);
int rnd(void *buf, size_t len, void *ctx);

int func_zero(int argc, char **argv);
int func_one(int argc, char **argv);
int func_push(int argc, char **argv);
int func_pop(int argc, char **argv);
int func_iszero(int argc, char **argv);
int func_isone(int argc, char **argv);
int func_isneg(int argc, char **argv);
int func_bits(int argc, char **argv);
int func_shr(int argc, char **argv);
int func_shl(int argc, char **argv);
int func_add(int argc, char **argv);
int func_sub(int argc, char **argv);
int func_mul(int argc, char **argv);
int func_div(int argc, char **argv);
int func_sqr(int argc, char **argv);
int func_gcd(int argc, char **argv);
int func_rnd(int argc, char **argv);
int func_cmp(int argc, char **argv);
int func_acmp(int argc, char **argv);
int func_swap(int argc, char **argv);
int func_primal(int argc, char **argv);
int func_barretts(int argc, char **argv);
int func_barrett(int argc, char **argv);
int func_minv(int argc, char **argv);
int func_mexp(int argc, char **argv);


mpl_int stack[STKSIZE];
unsigned int in_stack;
unsigned int places;
unsigned int strnum;

const char *errtab[] = {
	"0 cell is unused",
	"no memory\n",
	"wrong number format\n",
	"wrong number of arguments\n",
	"not enough stack arguments\n",
	"stack overflow\n",
	"wrong argument\n",
	"impossible situation\n",
	"unknown command\n"
};

struct function ftab[] = {
	{"zero",     0, 0, 1, func_zero},
	{"one",      0, 0, 1, func_one},
	{"push" ,    1, 0, 1, func_push},
	{"pop",      0, 1, 0, func_pop},
	{"iszero",   0, 1, 1, func_iszero},
	{"isone",    0, 1, 1, func_isone},
	{"isneg",    0, 1, 1, func_isneg},
	{"bits",     0, 1, 1, func_bits},
	{"shr",      1, 1, 1, func_shr},
	{"shl",      1, 1, 1, func_shl},
	{"add",      0, 2, 1, func_add},
	{"sub",      0, 2, 1, func_sub},
	{"mul",      0, 2, 1, func_mul},
	{"div",      0, 2, 2, func_div},
	{"sqr",      0, 1, 1, func_sqr},
	{"gcd",      0, 2, 1, func_gcd},
	{"rnd",      1, 0, 1, func_rnd},
	{"cmp",      0, 2, 2, func_cmp},
	{"acmp",     0, 2, 2, func_acmp},
	{"swap",     0, 2, 2, func_swap},
	{"primal",   1, 1, 1, func_primal},
	{"barretts", 0, 1, 1, func_barretts},
	{"barrett",  0, 3, 2, func_barrett},
	{"minv",     0, 2, 1, func_minv},
	{"mexp",     0, 3, 1, func_mexp}
};

int
main()
{
	char buf[BUFSIZE];

	stk_init();

	strnum = 0;
	while (fgets(buf, BUFSIZE, stdin) != NULL) {
		strnum++;
		if (process_input(buf, strlen(buf)) != OK)
			break;
	}

	stk_free();

	return 0;
}

void
stk_init(void)
{
	in_stack = 0;
	places   = 0;
}

void
stk_grow(int n) {
	in_stack += n;
}

void
stk_free(void)
{
	while (places-- > 0)
		mpl_clear(stack + places);

}

int
stk_nelems(void)
{
	return in_stack;
}

int
stk_places(void)
{
	return places;
}

int
stk_add_place(void)
{
	if (mpl_init(stack + places) != MPL_OK)
		return NO_MEMORY;

	places++;

	return OK;
}

mpl_int *
stk(int n)
{
	return (stack + in_stack - n - 1);
}

int
process_input(char *buf, int len)
{
	int i, argc, res, inword;
	char *argv[ARGVMAX], *end;

	for (i = 0; i < ARGVMAX; i++)
		argv[i] = NULL;

	if (len > 0) {
		if (buf[len-1] == '\n') {
			buf[len-1] = '\0';
			len--;
		}
	}

	if (len == 0)
		return 0;

	argc = 0;
	inword = 0;
	end = buf + len;
	while (buf <= end) {
		if (isblank(*buf) || *buf == '\0') {
			*buf = '\0';
			inword = 0;
		} else {
			if (argc < ARGVMAX && !inword) {
				argv[argc] = buf;
				argc++;
				inword++;
			}
		}
		buf++;
	}

	res = UNKNOWN;
	for (i = 0; i < ARRAY_SIZE(ftab); i++) {
		int gain, n;

		if (strcmp(ftab[i].name, argv[0]) != 0)
			continue;

		if (ftab[i].nargs != argc-1) {
			res = WRONG_NARGS;
			fprintf(stderr, "%i:%s: %s", strnum, ftab[i].name, errtab[res]);
			break;
		}

		n = stk_nelems();

		if (ftab[i].nin > n) {
			res = WRONG_STACK;
			fprintf(stderr, "%i:%s: %s", strnum, ftab[i].name, errtab[res]);
			break;
		}

		gain = ftab[i].nout - ftab[i].nin;

		if (n + gain > STKSIZE) {
			res = STACK_OVERFLOW;
			fprintf(stderr, "%i:%s: %s", strnum, ftab[i].name, errtab[res]);
			break;
		}

		/* if there is not enough mpl_init'ed places in stack, add them */
		while (n + gain > stk_places()) {
			res = stk_add_place();
			if (res != OK) {
				fprintf(stderr, "%i:%s: %s", strnum, ftab[i].name, errtab[res]);
				break;
			}
		}

		if ((res = ftab[i].func(argc, argv)) != OK)
			fprintf(stderr, "%i:%s: %s", strnum, ftab[i].name, errtab[res]);

		stk_grow(gain);

		break;
	}

	if (res == UNKNOWN)
		fprintf(stderr, "%i: %s", strnum, errtab[res]);


	return res;
}

int
urandom(void *buf, size_t len)
{
	int fd, res;

	fd = open("/dev/urandom", O_RDONLY);
	if (fd == -1)
		return -1;
	res = read(fd, buf, len);
	close(fd);
	return res;
}

int
rnd(void *buf, size_t len, void *ctx)
{
	return urandom(buf, len);
}

int
func_zero(int argc, char **argv)
{
	mpl_int *a;

	a = stk(-1);
	mpl_zero(a);

	return OK;
}

int
func_one(int argc, char **argv)
{
	mpl_int *a;

	a = stk(-1);
	mpl_set_one(a);

	return OK;
}

int
func_push(int argc, char **argv)
{
	mpl_int *a;
	int base, res;

	if (isdigit(argv[1][0]) || argv[1][0] == '-') {
		base = 10;
	} else {
		switch (argv[1][0]) {
		case 'o':
			base = 8;
			break;
		case 'd':
			base = 10;
			break;
		case 'x':
			base = 16;
			break;
		default:
			return WRONG_FORMAT;
			break;
		}
		argv[1]++;
	}

	a = stk(-1);
	res = mpl_set_str(a, argv[1], base);
	switch (res) {
	case MPL_NOMEM:
		res = NO_MEMORY;
		break;
	case MPL_ERR:
		res = WRONG_FORMAT;
		break;
	case MPL_OK:
		res = OK;
		break;
	default:
		res = IMPOSSIBLE;
		break;
	}

	return res;
}

int
func_pop(int argc, char **argv)
{
	mpl_int *a;
	int len, res;
	char *buf;

	a = stk(0);
	len = mpl_nr_bits(a)/4 + 3;

	buf = malloc(len*sizeof(*buf));
	if (buf == NULL)
		return NO_MEMORY;

	res = mpl_to_str(a, buf, 10, len);
	switch (res) {
	case MPL_NOMEM:
		res = NO_MEMORY;
		break;
	case MPL_ERR:
		res = WRONG_FORMAT;
		break;
	case MPL_OK:
		printf("%s\n", buf);
		res = OK;
		break;
	default:
		res = IMPOSSIBLE;
		break;
	}

	free(buf);

	return res;
}

int
func_iszero(int argc, char **argv)
{
	mpl_int *a;

	a = stk(0);
	printf("%i\n", mpl_iszero(a));

	return OK;
}

int
func_isone(int argc, char **argv)
{
	mpl_int *a;

	a = stk(0);
	printf("%i\n", mpl_isone(a));

	return OK;
}

int
func_isneg(int argc, char **argv)
{
	mpl_int *a;

	a = stk(0);
	printf("%i\n", mpl_isneg(a));

	return OK;
}

int
func_bits(int argc, char **argv)
{
	mpl_int *a;

	a = stk(0);
	printf("%i\n", mpl_nr_bits(a));

	return OK;
}

int
func_shr(int argc, char **argv)
{
	mpl_int *a;
	int nr;
	char *end;

	if (argv[1][0] == 'd')
		argv[1]++;

	if (argv[1][0] == '-') {
		argv[1]++;
		return func_shl(argc, argv);
	}

	if (!isdigit(argv[1][0]))
		return WRONG_FORMAT;

	nr = strtol(argv[1], &end, 10);
	if (*end != '\0')
		return WRONG_ARGUMENT;

	a = stk(0);
	if (mpl_shr(a, nr) != MPL_OK)
		return IMPOSSIBLE;

	return OK;
}

int
func_shl(int argc, char **argv)
{
	mpl_int *a;
	int nr;
	char *end;

	if (argv[1][0] == 'd')
		argv[1]++;

	if (argv[1][0] == '-') {
		argv[1]++;
		return func_shr(argc, argv);
	}

	if (!isdigit(argv[1][0]))
		return WRONG_FORMAT;

	nr = strtol(argv[1], &end, 10);
	if (*end != '\0')
		return WRONG_ARGUMENT;

	a = stk(0);
	if (mpl_shr(a, nr) != MPL_OK)
		return NO_MEMORY;

	return OK;
}

int
func_add(int argc, char **argv)
{
	mpl_int *a, *b;
	int res;

	a = stk(0);
	b = stk(1);

	/* c = a + b */
	res = mpl_add(b, a, b);
	switch (res) {
	case MPL_NOMEM:
		res = NO_MEMORY;
		break;
	case MPL_OK:
		res = MPL_OK;
		break;
	default:
		res = IMPOSSIBLE;
		break;
	}


	return res;
}

int
func_sub(int argc, char **argv)
{
	mpl_int *a, *b;
	int res;

	a = stk(0);
	b = stk(1);

	/* c = b - a */
	res = mpl_sub(b, b, a);
	switch (res) {
	case MPL_NOMEM:
		res = NO_MEMORY;
		break;
	case MPL_OK:
		res = MPL_OK;
		break;
	default:
		res = IMPOSSIBLE;
		break;
	}

	return res;
}

int
func_mul(int argc, char **argv)
{
	mpl_int *a, *b;
	int res;

	a = stk(0);
	b = stk(1);

	res = mpl_mul(b, a, b);
	switch (res) {
	case MPL_NOMEM:
		res = NO_MEMORY;
		break;
	case MPL_OK:
		res = MPL_OK;
		break;
	default:
		res = IMPOSSIBLE;
		break;
	}

	return res;
}

int
func_div(int argc, char **argv)
{
	mpl_int *a, *b;
	int res;

	a = stk(0);
	b = stk(1);

	res = mpl_div(b, a, b, a);
	switch (res) {
	case MPL_NOMEM:
		res = NO_MEMORY;
		break;
	case MPL_OK:
		res = MPL_OK;
		break;
	default:
		res = IMPOSSIBLE;
		break;
	}


	return res;
}

int
func_sqr(int argc, char **argv)
{
	mpl_int *a;
	int res;

	a = stk(0);
	res = mpl_sqr(a, a);
	switch (res) {
	case MPL_NOMEM:
		res = NO_MEMORY;
		break;
	case MPL_OK:
		res = MPL_OK;
		break;
	default:
		res = IMPOSSIBLE;
		break;
	}

	return res;
}

int
func_gcd(int argc, char **argv)
{
	mpl_int *a, *b;
	int res;

	a = stk(0);
	b = stk(1);

	res = mpl_gcd(b, a, b);
	switch (res) {
	case MPL_NOMEM:
		res = NO_MEMORY;
		break;
	case MPL_OK:
		res = MPL_OK;
		break;
	default:
		res = IMPOSSIBLE;
		break;
	}

	return res;
}

int
func_rnd(int argc, char **argv)
{
	mpl_int *a;
	int nr, res;
	char *end;

	if (argv[1][0] == 'd')
		argv[1]++;

	if (argv[1][0] == '-')
		return WRONG_ARGUMENT;

	if (!isdigit(argv[1][0]))
		return WRONG_FORMAT;

	nr = strtol(argv[1], &end, 10);
	if (*end != '\0')
		return WRONG_ARGUMENT;

	a = stk(-1);
	res = mpl_random(a, nr, rnd, NULL);
	switch (res) {
	case MPL_NOMEM:
		res = NO_MEMORY;
		break;
	case MPL_ERR:
		res = WRONG_FORMAT;
		break;
	case MPL_OK:
		res = MPL_OK;
		break;
	default:
		res = IMPOSSIBLE;
		break;
	}

	return res;
}

int
func_cmp(int argc, char **argv)
{
	mpl_int *a, *b;
	int res;

	a = stk(0);
	b = stk(1);

	res = mpl_cmp(a, b);
	switch (res) {
	case MPL_CMP_GT:
		res = 1;
		break;
	case MPL_CMP_EQ:
		res = 0;
		break;
	case MPL_CMP_LT:
		res = -1;
		break;
	default:
		return IMPOSSIBLE;
		break;
	}

	printf("%i\n", res);

	return OK;
}

int
func_acmp(int argc, char **argv)
{
	mpl_int *a, *b;
	int res;

	a = stk(0);
	b = stk(1);

	res = mpl_abs_cmp(a, b);
	switch (res) {
	case MPL_CMP_GT:
		res = 1;
		break;
	case MPL_CMP_EQ:
		res = 0;
		break;
	case MPL_CMP_LT:
		res = -1;
		break;
	default:
		return IMPOSSIBLE;
		break;
	}

	printf("%i\n", res);

	return OK;
}

int
func_swap(int argc, char **argv)
{
	mpl_int *a, *b;

	a = stk(0);
	b = stk(1);

	if (mpl_swap(a, b) != OK)
		return IMPOSSIBLE;

	return OK;
}

int
func_primal(int argc, char **argv)
{
	mpl_int *a;
	int nr, res;
	char *end;

	if (argv[1][0] == 'd')
		argv[1]++;

	if (argv[1][0] == '-')
		return WRONG_ARGUMENT;

	if (!isdigit(argv[1][0]))
		return WRONG_FORMAT;

	nr = strtol(argv[1], &end, 10);
	if (*end != '\0')
		return WRONG_ARGUMENT;

	a = stk(0);
	if ((res = mpl_primality_miller_rabin(a, nr, rnd, NULL)) != MPL_OK) {
		switch (res) {
		case MPL_NOMEM:
			res = NO_MEMORY;
			break;
		case MPL_ERR:
			res = WRONG_ARGUMENT;
			break;
		case MPL_COMPOSITE:
			printf("%i\n", 0);
			res = OK;
			break;
		default:
			res = IMPOSSIBLE;
			break;
		}
	} else {
		printf("%i\n", 1);
		res = MPL_OK;
	}

	return res;
}

int
func_barretts(int argc, char **argv)
{
	mpl_int *a;
	int res;

	a = stk(0);
	res = mpl_reduce_barrett_setup(a, a);
	switch (res) {
	case MPL_NOMEM:
		res = NO_MEMORY;
		break;
	case MPL_OK:
		res = MPL_OK;
		break;
	default:
		res = IMPOSSIBLE;
		break;
	}

	return res;
}

int
func_barrett(int argc, char **argv)
{
	mpl_int *a, *b, *mu;
	int res;

	mu = stk(2);
	a  = stk(1);
	b  = stk(0);

	res = mpl_reduce_barrett(a, a, b, mu);
	switch (res) {
	case MPL_NOMEM:
		res = NO_MEMORY;
		break;
	case MPL_OK:
		res = MPL_OK;
		break;
	default:
		res = IMPOSSIBLE;
		break;
	}

	return res;
}

int
func_minv(int argc, char **argv)
{
	mpl_int *a, *b;
	int res;

	a  = stk(1);
	b  = stk(0);
	
	res = mpl_mod_inv(a, a, b);
	switch (res) {
	case MPL_ERR:
		res = WRONG_ARGUMENT;
		break;
	case MPL_NOMEM:
		res = NO_MEMORY;
		break;
	case MPL_OK:
		res = OK;
		break;
	default:
		res = IMPOSSIBLE;
		break;
	}

	return res;
}

int
func_mexp(int argc, char **argv)
{
	mpl_int *a, *y, *b;
	int res;

	b = stk(2);
	y = stk(1);
	a = stk(0);

	res = mpl_mod_exp(b, a, y, b);
	switch (res) {
	case MPL_ERR:
		res = WRONG_ARGUMENT;
		break;
	case MPL_NOMEM:
		res = NO_MEMORY;
		break;
	case MPL_OK:
		res = OK;
		break;
	default:
		res = IMPOSSIBLE;
		break;
	}

	return res;
}

