GOSRCDIR = ${GOROOT}/src/xapian

all:
	${SWIG} ${SWIG_FLAGS} -c++ -go -cgo -intgosize 64 go.i
	mkdir -p ${GOSRCDIR}
	cp -r * ${GOSRCDIR}
	cd ${GOSRCDIR}
	${GO} env -w CGO_CFLAGS="-g -O2 -L${abs_top_srcdir}/../xapian-core/.libs/ -lxapian-1.5 ${XAPIAN_CXXFLAGS}"
	${GO} env -w CGO_CPPFLAGS="-g -O2 -L${abs_top_srcdir}/../xapian-core/.libs/ -lxapian-1.5 ${XAPIAN_CXXFLAGS}"
	${GO} env -w CGO_CXXFLAGS="-g -O2 -L${abs_top_srcdir}/../xapian-core/.libs/ -lxapian-1.5 ${XAPIAN_CXXFLAGS}"
	${GO} env -w CGO_FFLAGS="-g -O2 -L${abs_top_srcdir}/../xapian-core/.libs/ -lxapian-1.5 ${XAPIAN_CXXFLAGS}"
	${GO} env -w CGO_LDFLAGS="-g -O2 -L${abs_top_srcdir}/../xapian-core/.libs/ -lxapian-1.5 ${XAPIAN_CXXFLAGS}"     
	${GO} install -i -x -a
	cp ${abs_top_srcdir}/../xapian-core/.libs/libxapian-1.5.so.0.0.0 /usr/lib/ && ldconfig
	
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

modify : run
	mkdir -p ${GOSRCDIR}
	cp -r * ${GOSRCDIR}

run :  
	${SWIG} ${SWIG_FLAGS} -c++ -go -cgo -intgosize 64 go.i
	sed '17 i \#cgo pkg-config : xapian-core-1.5' xapian.go > xapian2.go
	mv xapian2.go xapian.go

check :
	cd ${GOSRCDIR}/test && ${GO} test
	${GO} env -w CGO_CFLAGS="-g -O2 "
	${GO} env -w CGO_CPPFLAGS=""
	${GO} env -w CGO_CXXFLAGS="-g -O2"
	${GO} env -w CGO_FFLAGS="-g -O2 "
	${GO} env -w CGO_LDFLAGS="-g -O2 "

.PHONY: build modify run check
