JDKINC = -I/usr/lib/jdk1.1/include -I/usr/local/jdk1.1/include/linux
INCLUDES = -I$(top_srcdir)/common -I$(top_srcdir)/include $(JDKINC)
#CPP     = g++

noinst_OBJECTS = libomusjava.so

JCLAS =	com.muscat.om.OmEnquire				\
	com.muscat.om.OmDatabaseGroup			\
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


JAVALIB_SOURCE = nvOmEnquire.cc			\
	nvOmDatabaseGroup.cc		\
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

libomusjava_so_OBJS = nvOmEnquire.lo			\
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

libomusjava.so: $(libomusjava_so_OBJS)
	$(LIBTOOL) $(CXX) -shared $(libomusjava_so_OBJS) ../../../libomus.la -o $@

#all:	$(MODS)
#	$(CPP) -shared -O3 -o libomusjava.so -lpthread $(MODS) $(OMLIB)

#.cc.o:
#	$(CPP) -fPIC -Wall -g $(JDKINC) $(OMINC) -c $<

headers:
	CLASSPATH=../classes javah -jni $(JCLAS)
