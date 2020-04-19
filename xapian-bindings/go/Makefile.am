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
	cd work && ${GO} run get_paths.go $(abs_top_builddir) ${USE_CLANG_GO_LIB}
	${AWK} 'NR==18{print "#cgo CXXFLAGS: ${XAPIAN_CXXFLAGS}" RS\
		"#cgo CXXFLAGS: ${CXXFLAGS}" RS\
		"#cgo CPPFLAGS: ${CPPFLAGS}"}1' xapian.go > xapian2.go
	mv xapian2.go xapian.go
	${GO} env -w CXX="${CXX}"
check :
	cd ${GOSRCDIR}/test && ${GO} test
clean :
	rm -r xapian.go
.PHONY: build modify run check
