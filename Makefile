CFLAGS   = -I include -Wall

headers  = mpl.h mpl_common.h

sources  = mpl_init.c mpl_clear.c mpl_util.c mpl_set.c mpl_ensure.c\
	   mpl_div.c mpl_add.c mpl_sub.c mpl_shift.c mpl_copy.c mpl_cmp.c\
	   mpl_to.c mpl_sqr.c mpl_gcd.c mpl_random.c mpl_primality.c\
	   mpl_reduce.c mpl_mod_exp.c mpl_mod_inv.c mpl_mul.c

objects  = mpl_init.o mpl_clear.o mpl_util.o mpl_set.o mpl_ensure.o\
	   mpl_div.o mpl_add.o mpl_sub.o mpl_shift.o mpl_copy.o mpl_cmp.o\
	   mpl_to.o mpl_sqr.o mpl_gcd.o mpl_random.o mpl_primality.o\
	   mpl_reduce.o mpl_mod_exp.o mpl_mod_inv.o mpl_mul.o

binaries = mplc

shared   = libmp.so

vpath %.c src
vpath %.h include

# binary targets
$(binaries): %: %.c $(objects)
	gcc $(CFLAGS) -o $@ $^

# object targets
$(objects): %.o: %.c $(headers)
	gcc $(CFLAGS) -fPIC -c -o $@ $<

# shared targets
$(shared): %.so: $(objects)
	gcc -shared -Wl,-soname,libmp.so -o $@ $^

# installation
.PHONY: install
install:
	cp $(shared) /usr/lib/
	cp include/* /usr/include

.PHONY: clean
clean:
	rm -f $(binaries) $(objects) $(shared)

