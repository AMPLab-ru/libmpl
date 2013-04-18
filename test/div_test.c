#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/mpl.h"

void
test_div(mpl_int *r, mpl_int *q, mpl_int *a, mpl_int *b)
{
	char buf[128];

	if (mpl_div(r, q, a, b)!= MPL_OK) {
		printf("error occured\n");
		exit(1);
	}

	if (r != NULL) {
		mpl_to_str(r, buf, 10, sizeof(buf));
		printf("reminder is %s\n", buf);
	}
	if (q != NULL) {
		mpl_to_str(q, buf, 10, sizeof(buf));
		printf("quoter is %s\n", buf);
	}
}

int 
main(int argc, const char *argv[])
{
	mpl_int a, b, c, d;

	mpl_initv(&a, &b, &c, &d, NULL);

	mpl_set_sint(&a, 444);
	mpl_set_sint(&b, 4);

	test_div(&c, &d, &a, &b);
	test_div(NULL, &d, &a, &b);
	test_div(&c, NULL, &a, &b);
	test_div(NULL, NULL, &a, &b);


	mpl_clearv(&a, &b, &c, &d, NULL);

	return 0;
}
