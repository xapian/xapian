GOSRCDIR = ${GOPATH}/src/xapian
TESTS = ./test
AM_TESTS_ENVIRONMENT = \
	abs_builddir='$(abs_builddir)' ;\
	srcdir='$(srcdir)'
	export abs_builddir ;\
	export srcdir ;
LOG_COMPILER = ${GO} test ./test

install-data-hook:
	cd ${GOSRCDIR} && ${GO} install -i -x -a

all : modify
	cd ${GOSRCDIR}
	${GO} build -i -x -a

modify : build
	${MKDIR_P} ${GOPATH}
	${MKDIR_P} ${GOPATH}/bin
	${MKDIR_P} ${GOPATH}/src
	${MKDIR_P} ${GOPATH}/pkg
	${MKDIR_P} ${GOSRCDIR}
	cp -r * ${GOSRCDIR}

build :
	${SWIG} ${SWIG_FLAGS} -c++ -go -cgo -intgosize 64 go.i
	${AWK} 'NR==18{print "#cgo CXXFLAGS: ${XAPIAN_CXXFLAGS}"}1' xapian.go > xapian2.go
	mv xapian2.go xapian.go
	${AWK} 'NR==19{print "#cgo CXXFLAGS: ${CXXFLAGS}"}1' xapian.go > xapian2.go
	mv xapian2.go xapian.go
	${AWK} 'NR==17{print "#cgo LDFLAGS: ${USE_CLANG_GO_LIB}"}1' xapian.go > xapian2.go
	mv xapian2.go xapian.go
	${AWK} 'NR==17{print "#cgo CPPFLAGS: ${CPPFLAGS}"}1' xapian.go > xapian2.go
	mv xapian2.go xapian.go
	${AWK} 'NR==17{print "#cgo LDFLAGS: -L${abs_top_srcdir}/../xapian-core/.libs -Wl,-rpath,${abs_top_srcdir}/../xapian-core/.libs -lxapian-1.5"}1' xapian.go > xapian2.go
	mv xapian2.go xapian.go
	${GO} env -w CXX="${CXX}"
check :
	cd ${GOSRCDIR}/test && ${GO} test
clean :
	rm -r xapian.go
.PHONY: build modify run check
