GOSRCDIR = ${GOROOT}/src/xapian
TESTS = ./test
AM_TESTS_ENVIRONMENT = \
	abs_builddir='$(abs_builddir)' ;\
	srcdir='$(srcdir)'
	export abs_builddir ;\
	export srcdir ;
LOG_COMPILER = ${GO} test ./test

install-data-hook: build

build : modify
	cd ${GOROOT}/src/xapian
	${GO} install -i -x -a
	ldconfig

modify :
	mkdir -p ${GOSRCDIR}
	cp -r * ${GOSRCDIR}

all :
	${SWIG} ${SWIG_FLAGS} -c++ -go -cgo -intgosize 64 go.i
	awk 'NR==17{print "#cgo pkg-config : xapian-core-1.5"}1' xapian.go > xapian2.go
	echo "export PKG_CONFIG_PATH=PKG_CONFIG_PATH:{abs_top_srcdir}/../xapian-core/pkg-config >> ~/.bashrc && source ~/.bashrc"
	echo "export PKG_CONFIG_PATH={abs_top_srcdir}/../xapian-core/pkg-config >> ~/.bashrc && source ~/.bashrc"
	PKG_CONFIG_PATH=${abs_top_srcdir}/../xapian-core/pkg-config
	mv xapian2.go xapian.go
	${GO} env -w CXX="${CXX}"
	${GO} env -w CGO_CPPFLAGS="${CPPFLAGS}"
check :
	cd ${GOSRCDIR}/test && ${GO} test

.PHONY: build modify run check
