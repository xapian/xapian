JDKINC  = -I/usr/local/jdk/include -I/usr/local/jdk/include/linux
OMINC   = -I../../om/common -I../../om/include -I../../om/include/om
#OMLIB   = ../../om/.libs/libomus.a
OMLIB   = -lomus
CPP     = g++

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


MODS  = nvOmEnquire.o			\
	nvOmDatabaseGroup.o		\
	nvOmQuery.o 			\
	nvOmMSet.o 			\
	nvOmMSetItem.o			\
	nvOmMSetItemVector.o		\
	nvOmDocument.o			\
	nvOmStem.o			\
	nvOmMatchOptions.o		\
	nvOmRSet.o			\
	nvOmESet.o			\
	nvOmESetItem.o			\
	nvOmESetItemVector.o		\
	nvOmExpandOptions.o		\
	nvOmPrefixExpandDecider.o	\
	utils.o

all:	$(MODS)
	$(CPP) -shared -O3 -o libomusjava.so -lpthread $(MODS) $(OMLIB)

.cc.o:
	$(CPP) -fPIC -Wall -g $(JDKINC) $(OMINC) -c $<

clean:	
	rm *.o *.so

cleaner:
	rm *.o *.so com_*

headers:
	CLASSPATH=../classes javah -jni $(JCLAS)

install:
	cp libomusjava.so /lib
	/sbin/ldconfig
