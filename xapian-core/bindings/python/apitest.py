#!/usr/bin/python
from omuscat import *

def get_simple_database():
    mydb = OmDatabaseGroup()
    mydb.add_database("sleepycat", ["/home/cemerson/working/open-muscat/build/om-debug-valis/tests/.sleepy/db=apitest_simpledata="])
    return mydb

def init_simple_enquire(enq, query = OmQuery("thi")):
    enq.set_query(query)

def do_get_simple_query_mset(query, maxitems = 10, first = 0):
    enquire = OmEnquire(get_simple_database())
    init_simple_enquire(enquire, query)

    return enquire.get_mset(first, maxitems)

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

def test_collapsekey1():
    success = 1

    enquire = OmEnquire(get_simple_database())
    init_simple_enquire(enquire)

    mymopt = OmMatchOptions()
    mymset1 = enquire.get_mset(0, 100, "NULL", mymopt);
    mymsize1 = len(mymset1.items)

    for key_no in range(1, 8):
	mymopt.set_collapse_key(key_no)
	mymset = enquire.get_mset(0, 100, "NULL", mymopt)

	if(mymsize1 <= len(mymset.items)):
	    success = 0;
	    print "Had no fewer items when performing collapse: don't know whether it worked."

	keys = {}
	for i in mymset.items:
	    key = enquire.get_doc(i[0]).get_key(key_no)
	    if (key != i[2]):
		success = 0;
		print "Expected key value was not found in MSetItem: ",
		print "expected `", key,
		print "' found `", i[2],
		print "'"

	    if (keys.has_key(key) and keys[key] != 0 and key != ""):
	        success = 0;
		print "docids", keys[key], "and", i[0], "both found in MSet with key `", key, "'"
		break
	    else:
	        keys[key] = i[0]
	if (not success):
	    break

    return success

def test_expandmaxitems1():
    enquire = OmEnquire(get_simple_database())
    init_simple_enquire(enquire)

    mymset = enquire.get_mset(0, 10)

    if (len(mymset.items) < 2):
        return 0;

    myrset = OmRSet()
    mymsetitems = mymset.items
    myrset.add_document(mymsetitems[0][0])
    myrset.add_document(mymsetitems[1][0])

    myeset = enquire.get_eset(1, myrset)

    return (len(myeset.items) == 1)

def test_batchquery1():
    success = 1
    verbose = 1

    myqueries = [
	( OmQuery("thi"), 0, 10, 0, 0, 0),
	( OmQueryNull(), 0, 10, 0, 0, 0),
	( OmQuery("word"), 0, 10, 0, 0, 0)]

    print myqueries

    benq = OmBatchEnquire(get_simple_database())

    print "foo"
    benq.set_queries(myqueries)

    print "bar"
    myresults = benq.get_msets()

    print "myresults = ", myresults
    if (len(myresults) != 3):
	success = 0;
	if (verbose):
	    print "Results size is", len(myresults), "expected 3."
	    print "result is: ", myresults
    else:
	if (myresults[0] != do_get_simple_query_mset(OmQuery("thi"))):
	    success = 0
	    if (verbose):
		print "Query 1 returned different result!"
		print "myresults[0] =", myresults[0]
		print "simple =", do_get_simple_query_mset(OmQuery("thi"))
	try:
 	    foo = myresults[1]
	    success = 0
	    if (verbose):
		print "Query 2 should not be valid"
	except:
	    pass

	if (myresults[2] != do_get_simple_query_mset(OmQuery("word"))):
	    success = 0
	    if (verbose):
		print "Query 3 returned different result!"

    return success
