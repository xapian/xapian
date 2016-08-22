noinst_HEADERS +=\
	cluster/points.h\
	cluster/similarity.h

EXTRA_DIST +=\
	cluster/Makefile

lib_src +=\
	cluster/cluster.cc\
	cluster/euclidian_sim.cc\
	cluster/round_robin.cc
