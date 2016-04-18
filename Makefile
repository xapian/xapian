
all:
	cd xapian-core && $(MAKE)
distcheck:
	cd xapian-core && $(MAKE) $@ && cp xapian*-core*.tar.xz ..
install:
	cd xapian-core && $(MAKE) $@

.PHONY: all distcheck install
