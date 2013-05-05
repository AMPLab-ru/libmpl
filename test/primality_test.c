#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "../include/mpl.h"
#include "common.h"

#define FALSE 0
#define TRUE !FALSE

void
test_prime(mpl_int *a, int num, int is_prime)
{
	int ret;
	int r;

	r = 1;
	
	mpl_set_sint(a, num);
	
	ret = mpl_primality_miller_rabin(a, r, rand_urandom, NULL);
	if (ret == MPL_ERR) {
		printf("error occured\n");
		return;
	}
	if ((ret == MPL_OK && is_prime) | 
	    (ret == MPL_COMPOSITE && !is_prime)) {
		printf("[+] primary check success\n");
	} else  {
		printf("[-] primary check FAIL\n"
		    "%d mpl_primary returns that it %s\n", num,
		    ret == MPL_COMPOSITE ? "composite" : "prime");
	}
}


int 
main(int argc, const char *argv[])
{
	mpl_int a;

	mpl_init(&a);

	test_prime(&a, 211, TRUE);
	test_prime(&a, 23, TRUE);
	test_prime(&a, 212, FALSE);
	test_prime(&a, 213, FALSE);
	test_prime(&a, 1, FALSE);

	mpl_clear(&a);

	return 0;
}
