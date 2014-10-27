EXTRA_DIST += \
	geospatial/Makefile

noinst_HEADERS +=\
	geospatial/geoencode.h

lib_src += \
	geospatial/geoencode.cc \
	geospatial/latlongcoord.cc \
	geospatial/latlong_distance_keymaker.cc \
	geospatial/latlong_metrics.cc \
	geospatial/latlong_posting_source.cc
