#!/usr/bin/python
from omuscat import *

foo = OmStem("english")

print foo.stem_word("giraffe")

bar = OmQuery("word", 1, 2)
print bar.get_description()

baz = OmQueryList(OM_MOP_OR, [ OmQuery("giraff"), OmQuery("lion") ])
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

