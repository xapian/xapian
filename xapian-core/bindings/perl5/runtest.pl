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

