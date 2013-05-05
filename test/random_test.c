#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/mpl.h"
#include "common.h"


void
test_random(mpl_int *r, int size)
{
	char buf[1024];

	if (mpl_random(r, size, rand_null, NULL) != MPL_OK) {
		printf("mpl_random fail\n");
		return;
	}
	mpl_to_str(r, buf, 16, sizeof(buf));
	printf("random num(%d bytes) is %s\n", size, buf);
}

int 
main(int argc, const char *argv[])
{
	mpl_int r;

	mpl_init(&r);

	test_random(&r, 1);
	test_random(&r, 1);
	
	test_random(&r, 2);
	test_random(&r, 2);

	test_random(&r, 3);
	test_random(&r, 3);

	test_random(&r, 4);
	test_random(&r, 4);

	test_random(&r, 5);
	test_random(&r, 5);

	test_random(&r, 6);
	test_random(&r, 6);

	test_random(&r, 7);
	test_random(&r, 7);

	test_random(&r, 8);
	test_random(&r, 8);

	mpl_clear(&r);
	return 0;
}
