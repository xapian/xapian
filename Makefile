
all:
	cd "$(PKG_NAME)" && $(MAKE)
distcheck:
	cd "$(PKG_NAME)" && $(MAKE) $@

.PHONY: all distcheck
