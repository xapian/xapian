TARGETS = xapimg.py _xapimg.so imgseek_wrap.cxx

CCSOURCES = imgseek.cc jpegloader.cc haar.cc
HSOURCES = imgseek.h jpegloader.h haar.h

XAP = ../../
INCLUDES = -I/usr/include/qt3/ -I$(XAP)xapian-core/common/ -I$(XAP)xapian-bindings/ -I$(XAP)xapian-bindings/python/ -I$(XAP)xapian-core/include/ -I$(XAP)xapian-core/include/xapian/

all: $(TARGETS)

xapianimgseek.so: $(CCSOURCES) $(HSOURCES)
	g++ -lxapian -shared -fPIC -o $@ $(INCLUDES) $(CCSOURCES) 

xapimg.py imgseek_wrap.cxx: imgseek.i
	swig -Wall -python -c++ $(INCLUDES) $<

_xapimg.so: imgseek_wrap.cxx $(CCSOURCES)
	g++ -lxapian -lqt-mt -ljpeg -I/usr/include/python2.5/ -shared -fPIC -o $@ -I/usr/include/python2.5 $(INCLUDES) $^ /usr/lib/python2.5/site-packages/xapian/_xapian.so

clean:
	rm -rf $(TARGETS)
