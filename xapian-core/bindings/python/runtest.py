#!/usr/bin/python
from omuscat import *
from apitest import *

foo = OmStem("english")

print foo.stem_word("giraffe")

bar = OmQuery("word", 1, 2)
print bar.get_description()

baz = OmQueryList(OM_MOP_FILTER, [ OmQuery("giraff"), OmQuery("lion") ])
print baz.get_description()

baz = OmQueryList(OM_MOP_NEAR, [ OmQuery("giraff"), OmQuery("lion") ])
print baz.get_description()

myquery = OmQueryList(OM_MOP_OR,
               [ OmQueryList(OM_MOP_AND,
	            [ OmQuery("one", 1, 1), OmQuery("three", 1, 3) ]),
	         OmQueryList(OM_MOP_OR,
	            [ OmQuery("four", 1, 4), OmQuery("two", 1, 2) ])])

terms = myquery.get_terms()
print "terms = ", terms

if (terms != [ "one", "two", "three", "four" ]):
    print "Incorrect term list."

dbgrp = OmDatabaseGroup()

dbgrp.add_database("sleepycat", ["/home/cemerson/working/open-muscat/build/om-debug-valis/tests/.sleepy/db=apitest_simpledata="])

enq = OmEnquire(dbgrp)

enq.set_query(OmQuery("word", 0, 2))

mset = enq.get_mset(0, 10);

print mset.items

match_terms = enq.get_matching_terms(mset.items[0][0])
print match_terms

print test_pctcutoff1()
print test_collapsekey1()
print test_expandmaxitems1()
