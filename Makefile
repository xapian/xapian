
all:
	cd xapian-core && $(MAKE)
distcheck:
	cd xapian-core && $(MAKE) $@
install:
	cd xapian-core && $(MAKE) $@

.PHONY: all distcheck install
