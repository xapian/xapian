#!/usr/bin/perl
use omuscat;

$foo = new OmStem("english");

print $foo->stem_word("giraffe") . "\n";

$bar = OmQuery->new_OmQueryTerm("word", 1, 2);
print $bar->get_description() . "\n";

$baz = OmQuery->new_OmQueryList($omuscat::OM_MOP_OR,
			    [ OmQuery->new_OmQueryTerm("giraff"),
			      OmQuery->new_OmQueryTerm("lion") ]);
print $baz->get_description() . "\n";

$myquery = OmQuery->new_OmQueryList($omuscat::OM_MOP_OR,
               [ OmQuery->new_OmQueryList($omuscat::OM_MOP_AND,
	            [ OmQuery->new_OmQueryTerm("one", 1, 1),
		      OmQuery->new_OmQueryTerm("three", 1, 3) ]),
	         OmQuery->new_OmQueryList($omuscat::OM_MOP_OR,
	            [ OmQuery->new_OmQueryTerm("four", 1, 4),
		      OmQuery->new_OmQueryTerm("two", 1, 2) ])]);

$terms = join(", ", @{$myquery->get_terms()});
print "terms = \`$terms'\n";

if ($terms ne "one, two, three, four") {
    print "Incorrect term list."
};

$db = new OmWritableDatabase("inmemory", []);

$doc = new OmDocumentContents();

print "foo1\n";
$doc->set_data("This is a document");
print "foo2\n";
$doc->add_posting("foo", 1);
$doc->add_posting("word", 1);
print "foo3\n";

$db->add_document($doc);
$dbgrp = new OmDatabaseGroup();
$dbgrp->add_database($db);

$enquire = new OmEnquire($dbgrp);

$enquire->set_query(OmQuery->new_OmQueryTerm("word"));
print $enquire->get_mset(0, 10)->get_description(), "\n";
