
all:
	cd xapian-core && $(MAKE)
distcheck:
	cd xapian-core && $(MAKE) $@

.PHONY: all distcheck
