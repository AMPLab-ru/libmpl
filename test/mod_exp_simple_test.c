#include <stdint.h>
#include <stdio.h>

#include "../include/mpl.h"

int 
main(int argc, const char *argv[])
{
	mpl_int a, b, c, d;
	char buf[128];

	mpl_initv(&a, &b, &c, &d, NULL);

	mpl_set_sint(&a, 0);
	mpl_set_sint(&b, 0);
	mpl_set_sint(&c, 0);

	if (mpl_mod_exp_simple(&d, &a, &b, &c) != MPL_OK) {
		printf("error occured\n");
		exit(1);
	}
	mpl_to_str(&d, buf, 10, sizeof(buf));

	printf("mod_exp_simple ans is %s\n", buf);

	if (mpl_mod_exp(&d, &a, &b, &c) != MPL_OK) {
		printf("error occured\n");
		exit(1);
	}
	mpl_to_str(&d, buf, 10, sizeof(buf));
	printf("mod_exp ans is %s\n", buf);

	mpl_clearv(&a, &b, &c, &d, NULL);

	return 0;
}
