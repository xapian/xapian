
all:
	cd xapian-core && $(MAKE)
distcheck:
	cd xapian-core && $(MAKE) $@ && cp xapian-core*.tar.xz ..

.PHONY: all distcheck
