pages   = mpl.3.gz

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

