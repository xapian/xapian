JDKINC = -I/usr/lib/jdk1.1/include -I/usr/local/jdk1.1/include/linux
INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include $(JDKINC)
#CPP     = g++

JCLAS =	com.muscat.om.OmEnquire				\
	com.muscat.om.OmDatabaseGroup			\
	com.muscat.om.OmDatabase			\
	com.muscat.om.OmQuery				\
	com.muscat.om.OmMSet				\
	com.muscat.om.OmMSetItem			\
	com.muscat.om.OmMSetItemVector			\
	com.muscat.om.OmStem				\
	com.muscat.om.OmDocument			\
	com.muscat.om.OmMatchOptions    		\
	com.muscat.om.OmRSet				\
	com.muscat.om.OmESet				\
	com.muscat.om.OmESetItem			\
	com.muscat.om.OmESetItemVector			\
	com.muscat.om.OmExpandOptions			\
	com.muscat.om.OmExpandDecider			\
	com.muscat.om.OmPrefixExpandDecider

check_PROGRAMS = dummy

dummy_SOURCES = dummy.cc

EXTRA_DIST = nvOmEnquire.cc			\
	nvOmDatabaseGroup.cc		\
	nvOmDatabase.cc			\
	nvOmQuery.cc 			\
	nvOmMSet.cc 			\
	nvOmMSetItem.cc			\
	nvOmMSetItemVector.cc		\
	nvOmDocument.cc			\
	nvOmStem.cc			\
	nvOmMatchOptions.cc		\
	nvOmRSet.cc			\
	nvOmESet.cc			\
	nvOmESetItem.cc			\
	nvOmESetItemVector.cc		\
	nvOmExpandOptions.cc		\
	nvOmPrefixExpandDecider.cc	\
	utils.cc

native_sources = nvOmEnquire.lo			\
	nvOmDatabaseGroup.lo		\
	nvOmQuery.lo 			\
	nvOmMSet.lo 			\
	nvOmMSetItem.lo			\
	nvOmMSetItemVector.lo		\
	nvOmDocument.lo			\
	nvOmStem.lo			\
	nvOmMatchOptions.lo		\
	nvOmRSet.lo			\
	nvOmESet.lo			\
	nvOmESetItem.lo			\
	nvOmESetItemVector.lo		\
	nvOmExpandOptions.lo		\
	nvOmPrefixExpandDecider.lo	\
	utils.lo

all-local: headers libomusjava.so

libomusjava.so: $(native_sources) ../../../libomus.la
	$(LIBTOOL) $(CXX) -shared $^ -o $@

#all:	$(MODS)
#	$(CPP) -shared -O3 -o libomusjava.so -lpthread $(MODS) $(OMLIB)

#.cc.o:
#	$(CPP) -fPIC -Wall -g $(JDKINC) $(OMINC) -c $<

headers:
	CLASSPATH=../classes javah -jni $(JCLAS)
