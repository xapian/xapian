#!/usr/bin/python
from omuscat import *
from apitest import *

foo = OmStem("english")

print foo.stem_word("giraffe")

bar = OmQuery("word", 1, 2)
print bar.get_description()

baz = OmQueryList(OM_MOP_FILTER, [ OmQuery("giraff"), OmQuery("lion") ])
print baz.get_description()

baz = OmQueryList(OM_MOP_NEAR, [ OmQuery("giraff"), OmQuery("lion") ], 3)
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

tests = [("pctcutoff1", test_pctcutoff1),
         ("collapsekey1", test_collapsekey1),
	 ("expandmaxitems1", test_expandmaxitems1),
	 ("batchquery1", test_batchquery1)]

global verbose
verbose = 0

for test in tests:
    print test[0] + "...",
    if test[1]():
        print "ok"
    else:
        print "FAILED"
