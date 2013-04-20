#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/mpl.h"

//mod_exp(1000000, 100, 455)

void
test_mod_exp(mpl_int *d, mpl_int *a, mpl_int *b, mpl_int *c)
{
	char buf[128];
	if (mpl_mod_exp(d, a, b, c) != MPL_OK) {
		printf("mpl_mod_exp error occured\n");
	}
	mpl_to_str(d, buf, 10, sizeof(buf));

	printf("mod_exp ans is %s\n", buf);

	if (mpl_mod_exp_simple(d, a, b, c) != MPL_OK) {
		printf("mpl_mod_exp_simple error occured\n");
	}
	mpl_to_str(d, buf, 10, sizeof(buf));

	printf("mod_exp ans is %s\n", buf);
}

int 
main(int argc, const char *argv[])
{
	mpl_int a, b, c, d;

	mpl_initv(&a, &b, &c, &d, NULL);

	mpl_set_sint(&a, 1000000);
	mpl_set_sint(&b, 1);
	mpl_set_sint(&c, 455);

	test_mod_exp(&d, &a, &b, &c);

	mpl_clearv(&a, &b, &c, &d, NULL);

	return 0;
}
