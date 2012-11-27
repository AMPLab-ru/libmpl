pages   = mpl_init.3.gz mpl_initv.3.gz mpl_clear.3.gz mpl_clearv.3.gz\
	  mpl_zero.3.gz mpl_iszero.3.gz mpl_isone.3.gz mpl_iseven.3.gz\
	  mpl_isodd.3.gz mpl_isneg.3.gz mpl_set_one.3.gz mpl_div.3.gz \
	  mpl_set_uint.3.gz mpl_set_uchar.3.gz mpl_shr.3.gz mpl_shl.3.gz\
	  mpl_set_str.3.gz mpl_to_uchar.3.gz mpl_to_str.3.gz mpl_add.3.gz\
	  mpl_nr_bits.3.gz mpl_sub.3.gz mpl_mul.3.gz mpl_set_sint.3.gz\
	  mpl_mul_dig.3.gz mpl_mul_ndig.3.gz mpl_sqr.3.gz mpl_gcd.3.gz\
	  mpl_random.3.gz mpl_primality_miller_rabin.3.gz mpl_cmp.3.gz\
	  mpl_reduce_barrett.3.gz mpl_and.3.gz mpl_xor.3.gz mpl_dbg.3.gz\
	  mpl_mod_inv.3.gz mpl_mod_exp.3.gz mpl_abs_cmp.3.gz mpl_or.3.gz\
	  mpl_swap.3.gz mpl_copy.3.gz mpl_reduce_barrett_setup.3.gz\

man3dir = /usr/share/man/man3/

all: $(pages)

%.3.gz: %.3
	gzip $<

%.3: %.man
	cp $< $@

.PHONY: install
install:
	mv $(pages) $(man3dir)

.PHONY: remove
remove:
	cd $(man3dir); rm -f $(pages)

.PHONY: clean
clean:
	rm -f $(pages)

