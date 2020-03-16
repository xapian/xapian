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
modify : run 
	mkdir -p ${GOSRCDIR}
	cp -r * ${GOSRCDIR}
run :  	
	swig ${XAPIAN_CXXFLAGS} -c++ -go -cgo -intgosize 64 go.i
	sed '17 i \#cgo pkg-config : xapian-core-1.5' xapian.go > xapian2.go
	mv xapian2.go xapian.go
check : build
	${GO} test ./test
.PHONY: build modify run check
