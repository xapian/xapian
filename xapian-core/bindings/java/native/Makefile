JDKINC  = -I/usr/local/jdk/include -I/usr/local/jdk/include/linux
OMINC   = -I../../om/common -I../../om/include -I../../om/include/om
#OMLIB   = ../../om/.libs/libomus.a
OMLIB   = -lomus
CPP     = g++

JCLAS =	com.webtop.om.OmEnquire				\
	com.webtop.om.OmDatabaseGroup			\
	com.webtop.om.OmQuery				\
	com.webtop.om.OmMSet				\
	com.webtop.om.OmMSetItem			\
	com.webtop.om.OmMSetItemVector			\
	com.webtop.om.OmStem				\
	com.webtop.om.OmDocument			\
	com.webtop.om.OmMatchOptions    		\
	com.webtop.om.OmRSet				\
	com.webtop.om.OmESet				\
	com.webtop.om.OmESetItem			\
	com.webtop.om.OmESetItemVector			\
	com.webtop.om.OmExpandOptions			\
	com.webtop.om.OmExpandDecider			\
	com.webtop.om.OmPrefixExpandDecider


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
