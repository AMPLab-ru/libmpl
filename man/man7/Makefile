pages   = mpl.7.gz

man7dir = /usr/share/man/man7/

all: $(pages)

%.7.gz: %.7
	gzip $<

%.7: %.man
	cp $< $@

.PHONY: install
install:
	install $(pages) $(man7dir)

.PHONY: uninstall
uninstall:
	cd $(man7dir); rm -f $(pages)

.PHONY: clean
clean:
	rm -f $(pages)

