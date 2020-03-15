GOSRCDIR = ${GOROOT}/src/xapian
install : build
build : modify
	cd ${GOROOT}/src/xapian
	${GO} install -i -x -a
modify : run 
	mkdir -p ${GOSRCDIR}
	cp -r * ${GOSRCDIR}
run : go.i 	
	swig ${XAPIAN_CXXFLAGS} -c++ -go -cgo -intgosize 64 go.i
	sed '17 i \#cgo pkg-config : xapian-core-1.5' xapian.go > xapian2.go
	mv xapian2.go xapian.go
