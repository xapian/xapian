#!/usr/bin/python
from omuscat import *

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

def get_simple_database():
    mydb = OmDatabaseGroup()
    mydb.add_database("sleepycat", ["/home/cemerson/working/open-muscat/build/om-debug-valis/tests/.sleepy/db=apitest_simpledata="])
    return mydb

def init_simple_enquire(enq):
    enq.set_query(OmQuery("thi"))

def print_mset_percentages(mset):
    items = mset.items
    for i in range(len(items)):
        print " ",
	print mset.convert_to_percent(items[i][1]),

def test_pctcutoff1():
    success = 1

    enquire = OmEnquire(get_simple_database())
    init_simple_enquire(enquire)

    mymset1 = enquire.get_mset(0, 100)

    print "Original mset pcts:",
    print_mset_percentages(mymset1)
    print

    num_items = 0;
    my_pct = 100;
    changes = 0;
    mymset1items = mymset1.items
    for i in range(len(mymset1items)):
        new_pct = mymset1.convert_to_percent(mymset1items[i][1]);
        if (new_pct != my_pct):
	    changes = changes + 1
	    if (changes <= 3):
	        num_items = i
		my_pct = new_pct

    if (changes <= 3):
	success = 0
	print "MSet not varied enough to test"

    print "Cutoff percent: ", my_pct
    
    mymopt = OmMatchOptions()
    mymopt.set_percentage_cutoff(my_pct)
    mymset2 = enquire.get_mset(0, 100, "NULL", mymopt)

    print "Percentages after cutoff:",
    print_mset_percentages(mymset2)
    print
    
    mymset2items = mymset2.items
    if (len(mymset2items) < num_items):
        success = 0
	print "Match with % cutoff lost too many items"
    elif (len(mymset2items) > num_items):
        for i in range(num_items, len(mymset2items)):
	    if (mymset2.convert_to_percent(mymset2.items[i]) != my_pct):
	        success = 0
		print "Match with % cutoff returned  too many items"
		break

    return success

print test_pctcutoff1()
