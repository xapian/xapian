# ToJava.pm: module to output Java code from the ad-hoc apitest parser.
#
# ----START-LICENCE----
# Copyright 1999,2000 BrightStation PLC
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA
# -----END-LICENCE-----

use strict;
use Carp;
use apitest_parser;

package ToJava;

BEGIN {
    %ToJava::typemap = ( "bool" => "boolean",
                         "unsigned int" => "int",
                         "unsigned" => "int",
                         "size_t" => "int",
                         "string" => "String",
                         "std::string" => "String",
			 "om_weight" => "double");
    %ToJava::basic_types = ("boolean" => 1,
                            "int" => 1,
			    "double" => 1);
}

sub new {
    my $class = shift;
    return bless { tests => "", func => ""}, $class;
}

sub get_value {
    my $self = shift;
    return $self->{func};
}

sub preamble() {
    return <<'EOF'
import java.lang.*;
import ApiTest.*;
import com.muscat.om.*;

class ApiTestFuncs {
    public static OmDatabase get_database(String arg) throws Throwable {
        return ApiTest.get_database(arg);
    }
    public static OmDatabase get_database(String arg1, String arg2) throws Throwable {
        return ApiTest.get_database(arg1, arg2);
    }
    private static void print_mset_percentages(OmMSet mset) throws Throwable {
        ApiTest.print_mset_percentages(mset);
    }
    public static OmEnquire get_simple_database() throws Throwable {
	OmDatabase mydbgrp = get_database("apitest_simpledata");
	return new OmEnquire(mydbgrp);
    }
    private static OmEnquire make_dbgrp(OmDatabase db1) throws Throwable {
        return new OmEnquire(ApiTest.make_dbgrp(db1));
    }
    private static OmEnquire make_dbgrp(OmDatabase db1,
                                        OmDatabase db2) throws Throwable {
        return new OmEnquire(ApiTest.make_dbgrp(db1, db2));
    }
    private static OmEnquire make_dbgrp(OmDatabase db1,
                                        OmDatabase db2,
					OmDatabase db3) throws Throwable {
        return new OmEnquire(ApiTest.make_dbgrp(db1, db2, db3));
    }
    private static OmMSet do_get_simple_query_mset(OmQuery query,
                                               int maxitems,
					       int first) throws Throwable {
	return ApiTest.get_simple_query_mset(query, maxitems, first);
    }
    private static OmMSet do_get_simple_query_mset(OmQuery query,
                                                   int maxitems) throws Throwable {
	return do_get_simple_query_mset(query, maxitems, 0);
    }
    private static OmMSet do_get_simple_query_mset(OmQuery query) throws Throwable {
	return do_get_simple_query_mset(query, 10, 0);
    }
    private static void init_simple_enquire(OmEnquire enq, OmQuery query) throws Throwable {
    	ApiTest.init_simple_enquire(enq, query);
    }
    public static void init_simple_enquire(OmEnquire enq) throws Throwable {
    	ApiTest.init_simple_enquire(enq);
    }
    private static boolean floats_are_equal_enough(double a, double b)
    {
	if (Math.abs(a - b) > 1E-5) return false;
	return true;
    }
    private static boolean weights_are_equal_enough(double a, double b) {
	if (floats_are_equal_enough(a, b)) return true;

	if(verbose) {
	    System.out.println("Got weight of " + a +
	                       ", expected weight of " + b);
	}
	return false;
    }
    private static boolean mset_equal(OmMSet first, OmMSet second) throws Throwable {
        if ((first.get_mbound() != second.get_mbound()) ||
	    (first.get_max_possible() != second.get_max_possible()) ||
	    (first.get_items().size() != second.get_items().size())) {
	    return false;
	}
	if (first.get_items().size() == 0) {
	    return true;
	}
	return mset_range_is_same(first, 0, second, 0, first.get_items().size());
    }
    private static void TEST_AND_EXPLAIN(boolean test, String message) throws Throwable {
        if (!test) {
	    throw new TestFail();
	}
    }
    private static void TEST_EQUAL(OmMSet mset1, OmMSet mset2) throws Throwable {
        TEST_AND_EXPLAIN(mset_equal(mset1, mset2),
	                 "Expected msets to be equal: were " +
			 mset1.get_description() + " and " +
			 mset2.get_description() + "\n");
    }
    private static void TEST_EQUAL(double d1, double d2) throws Throwable {
        TEST_AND_EXPLAIN(d1 == d2,
	                 "Expected doubles to be equal: were " +
			 d1 + " and " + d2 + "\n");
    }
    private static void TEST_NOT_EQUAL(double d1, double d2) throws Throwable {
        TEST_AND_EXPLAIN(d1 != d2,
	                 "Expected doubles to be different: were " +
			 d1 + " and " + d2 + "\n");
    }
    private static boolean TEST_EXPECTED_DOCS(OmMSet A,
    					      int d1,
					      int d2,
					      int d3) throws Throwable {
	int[] expect = new int[3];
	int expsize = 3;
	expect[0] = d1;
	expect[1] = d2;
	expect[2] = d3;

        return do_TEST_EXPECTED_DOCS(A, expect, expsize);
    }
    private static boolean TEST_EXPECTED_DOCS(OmMSet A,
    					      int d1,
					      int d2,
					      int d3,
					      int d4) throws Throwable {
	int[] expect = new int[4];
	int expsize = 4;
	expect[0] = d1;
	expect[1] = d2;
	expect[2] = d3;
	expect[3] = d4;

        return do_TEST_EXPECTED_DOCS(A, expect, expsize);
    }
    private static boolean do_TEST_EXPECTED_DOCS(OmMSet A,
    						 int[] expect,
						 int expsize) throws Throwable {
	OmVector A_items = A.get_items();
	if (A_items.size() != expsize) {
	    if (verbose) {
	        System.out.println("Match set is of wrong size: was " +
		                   A_items.size() + " - expected " + expsize);
		System.out.println("Full mset was: " + A.get_description());
	    }
	    return false;
	} else {
	    for (int i=0; i<A_items.size(); ++i) {
	        if (expect[i] != ((OmMSetItem)A_items.elementAt(i)).get_did()) {
		    if (verbose) {
			System.out.println("Match set didn't contain expected result:");
			System.out.println("Found docid " +
			                   ((OmMSetItem)A_items.elementAt(i)).get_did()
					   + " expected " + expect[i]);
			System.out.println("Full mset was: " + A.get_description());
		    }
		    return false;
		}
	    }
	}
	return true;
    }
    private static boolean mset_range_is_same_weights(OmMSet mset1, int first1,
    						      OmMSet mset2, int first2,
						      int count) throws Throwable {
	OmVector mset1_items = mset1.get_items();
	OmVector mset2_items = mset2.get_items();
	TEST_AND_EXPLAIN(mset1_items.size() >= first1 + count - 1,
	                 "mset1 is too small: expected at least " +
			 (first1 + count - 1) + " items.\n");

	TEST_AND_EXPLAIN(mset2_items.size() >= first2 + count - 1,
	                 "mset2 is too small: expected at least " +
			 (first2 + count - 1) + " items.\n");

	for (int i=0; i<count; ++i) {
	    if (((OmMSetItem)mset1_items.elementAt(i)).get_wt() !=
	        ((OmMSetItem)mset2_items.elementAt(i)).get_wt()) {
		return false;
	    }
	}
	return true;
    }
    private static boolean mset_range_is_same(OmMSet mset1, int first1,
                                              OmMSet mset2, int first2,
					      int count) throws Throwable {
	OmVector mset1_items = mset1.get_items();
	OmVector mset2_items = mset2.get_items();

	TEST_AND_EXPLAIN(mset1_items.size() >= first1 + count - 1,
			 "mset1 is too small: expected at least " +
			 (first1 + count - 1) + " items.\n");

	TEST_AND_EXPLAIN(mset2_items.size() >= first2 + count - 1,
			 "mset2 is too small: expected at least " +
			 (first2 + count - 1) + " items.\n");

	for (int i=0; i<count; ++i) {
	    if ((((OmMSetItem)mset1_items.elementAt(first1+i)).get_wt() !=
	         ((OmMSetItem)mset2_items.elementAt(first2+i)).get_wt()) || 
	        (((OmMSetItem)mset1_items.elementAt(first1+i)).get_did() !=
	         ((OmMSetItem)mset2_items.elementAt(first2+i)).get_did())) {
	        return false;
	    }
	}
	return true;
    }
    private static boolean verbose = false;
    private static String endl = "\n";
EOF
}

sub prologue() {
    my $self = shift;
    my $result = <<MAINSTART;
    public static void main(String[] argv) throws Throwable {
        String test = null;
        for (int i=0; i<argv.length; ++i) {
	    if (argv[i].equals("-v")) {
	        verbose = true;
	    } else {
	        if (test == null) {
		    test = argv[i];
		} else {
		    System.err.println("syntax: ApiTestFuncs [-v] [single test]");
		}
	    }
	}
        ApiTest.init();

	int succeeded = 0;
	int failed = 0;
MAINSTART
    my $name;
    foreach $name (split(/ +/, $self->{tests})) {
        if ($name eq "" or $name eq "test_alwaysfail") { next};
        $result .= <<TESTEND;
    if ((test == null) || (test.equals("$name"))) {
	System.out.print("${name}... ");
	try {
	    boolean result = ${name}();
	    if (result) {
		System.out.println("ok.");
		succeeded++;
	    } else {
		System.out.println("FAIL");
		failed++;
	    }
	} catch (TestFail foo) {
	    System.out.println("FAIL");
	}
    }
TESTEND
    }
    $result .= <<MAINEND
        System.out.println(succeeded + " tests passed, " +
	                   failed + " tests failed.");
    }
}
MAINEND
}

sub make_comment($) {
    my ($self, $text) = @_;

    return "/* $text */\n";
}

sub func_start($) {
    my $self = shift;
    my $orig = shift;
    if ($orig =~ /^bool ([a-z0-9_]+)\(\)/) {
        my $name = $1;
	$self->{func} = "    public static boolean $name() throws Throwable \n    {\n";
	if (defined($self->{indent_level}) and $self->{indent_level} != 4) {
	    $self->{tests} =~ s/test_[a-z0-9]+$//;
	}
	$self->{tests} .= " $name";
	$self->{indent_level} = 8;
	return $self->{func};
    } else {
        Carp::croak("Don't understand function header");
    }
}

sub map_type($) {
    my $orig_type = shift;
    if (exists $ToJava::typemap{$orig_type}) {
        return $ToJava::typemap{$orig_type};
    } else {
        return $orig_type;
    }
}

sub addtext($) {
    my ($self, $text) = @_;

    # do a few simple transformations to make it closer to java...
    #
    # remove address-of operator
    $text =~ s/\&([a-z]+)/$1/g;
    # const type & -> type
    $text =~ s/(?:const )($apitest_parser::type) (?:\&)/$1/g;
    # Change "OmType(foo)" -> "new OmType(foo)"
    $text =~ s/(?:(?<!new ))($apitest_parser::type)\(/new $1(/gs;
    # Change "mymsetfoo.items" -> "mymsetfoo_items" (set up in var_decl)
    $text =~ s/(mymset[a-z0-9]*)\.(items)/$1_$2/g;
    $text =~ s/(mymset[a-z0-9]*_items)\[([^\]])+\]\.([a-z]+)/((OmMSetItem)($1.elementAt($2))).get_$3()/g;
    $text =~ s/(mymset[a-z0-9]*_items)\[([^\]])+\](?!\.)/((OmMSetItem)($1.elementAt($2)))/g;
    # Change "mymset.membervar" to "mymset.get_membervar()"
    $text =~ s/(mymset[a-z0-9]*)\.([a-z_]+)(?=[^a-z0-9A-Z_.(])/$1.get_$2()/g;
    # Change "myesetfoo.items" -> "myesetfoo_items" (set up in var_decl)
    $text =~ s/(myeset[a-z0-9]*)\.(items)/$1_$2/g;
    $text =~ s/(myeset[a-z0-9]*_items)\[([^\]])+\]\.([a-z_]+)/((OmESetItem)($1.elementAt($2))).get_$3()/g;
    $text =~ s/(myeset[a-z0-9]*_items)\[([^\]])+\](?!\.)/((OmESetItem)($1.elementAt($2)))/g;
    # Change OM_MOP_FOO into "FOO"
    $text =~ s/OM_MOP_([A-Z_]+)/"$1"/g;
    # Change NULL into null
    $text =~ s/\bNULL\b/null/g;
    # Convert (mset1 == mset2) (which doesn't work with java references)
    # into mset_equal(mset1, mset2).
    $text =~ s/\b([a-z_]*mset[a-zA-Z0-9_]*) == ([a-z_]*mset[a-zA-Z0-9_]*)\b/mset_equal($1,$2)/g;
    # similarly for mset1 != mset2
    $text =~ s/\b([a-z_]*mset[a-zA-Z0-9_]*) != ([a-z_]*mset[a-zA-Z0-9_]*)\b/!mset_equal($1,$2)/g;
    # handle TEST_EXCEPTION here, as Java has no preprocessor
    if ($text =~ /^TEST_EXCEPTION\((Om[a-zA-Z]*Error), *(.*)\);$/) {
        my ($except, $code) = ($1, $2);
        $text = <<EOINLINE
try {
    $code;
    return false;
} catch ($except unused) { }
EOINLINE
    }

    $self->{func} .= (" " x $self->{indent_level}) . $text;
}

sub indent() {
    my $self = shift;
    $self->{indent_level} += 4;
}

sub undent() {
    my $self = shift;
    $self->{indent_level} -= 4;
}

sub var_decl($$$) {
    my ($self, $type, $name, $initialiser) = @_;
    $type = map_type($type);
    my $text;
    if (defined $initialiser) {
        if ($initialiser =~ /^($apitest_parser::func)(\(.*\))$/) {
	    my ($func, $args) = ($1, $2);
	    if (defined $args) {
		$text = "$type $name = $func$args;\n";
	    } else {
		$text = "$type $name = $func;\n";
	    }
	} else {
	    if (exists($ToJava::basic_types{$type})) {
		$text = "$type $name = $initialiser;\n";
	    } else {
		$text = "$type $name = new $type($initialiser);\n";
	    }
	}
    } else {
	$text = "$type $name = new $type();\n";
    }
    # some special cases: Om[EM]Set
    if ($type eq "OmMSet") {
        $text .= "OmVector ${name}_items = ${name}.get_items();\n";
    }
    if ($type eq "OmESet") {
        $text .= "OmVector ${name}_items = ${name}.get_items();\n";
    }

    $self->addtext($text);
    return $text;
}

sub func_call($$) {
    my ($self, $func, $args) = @_;

    my $text = "$func($args);\n";
    $self->addtext($text);
    return $text;
}

sub do_if($) {
    my ($self, $cond) = @_;

    my $text = "if ($cond) {\n";
    $self->addtext($text);
    $self->indent();
    return $text;
}

sub do_else() {
    my $self = shift;

    my $text = "} else {\n";
    $self->addtext($text);
    return $text;
}

sub do_elsif($) {
    my ($self, $cond) = @_;

    my $text = "} else if ($cond) {\n";
    $self->undent();
    $self->addtext($text);
    $self->indent();
    return $text;
}

sub do_try() {
    my $self = shift;
    my $text = "try {\n";
    $self->addtext($text);
    $self->indent();
    return $text;
}

sub do_catch($) {
    my ($self, $expt) = @_;

    # give a parameter name if not present
    if ($expt =~ s/(?:const )?($apitest_parser::type )(?:\&)?$/$1/) {
        $expt .= "unused_exception_type";
    }
    my $text = "} catch ($expt) {\n";
    $self->undent();
    $self->addtext($text);
    $self->indent();
    return $text;
}

sub do_for($$$) {
    my ($self, $precommand, $cond, $inc) = @_;

    if ($precommand =~ /^($apitest_parser::type) ($apitest_parser::identifier.*)/) {
        my ($type, $rest) = ($1, $2);
        $precommand = map_type($type) . " $rest"; 
    }
    
    my $text = "for ($precommand;$cond;$inc) {\n";
    $self->addtext($text);
    $self->indent();
    return $text;
}

sub close_block() {
    my $self = shift;
    my $text = "}\n";
    $self->undent();
    $self->addtext($text);
    return $text;
}

sub do_blank() {
    my $self = shift;
    my $text = "\n";
    # don't want the indenting here.
    $self->{func} .= $text;
    return $text;
}

sub do_invalid($) {
    my ($self, $line) = @_;
    my $text = "#INVALID:$line";
    $self->addtext($text);
    die "Can't handle invalid line";
    return $text;
}

sub do_return($) {
    my ($self, $val) = @_;
    my $text = "return $val;\n";
    $self->addtext($text);
    return $text;
}

sub do_break() {
    my $self = shift;
    my $text = "break;\n";
    $self->addtext($text);
    return $text;
}

sub do_comment($) {
    my ($self, $comment) = @_;
    my $text = "//$comment\n";
    $self->addtext($text);
    return $text;
}

sub do_cout(@$) {
    my ($self, $coutargs, $endl) = @_;
    my $text = ($endl? "System.out.println(" : "System.out.print(");
    $text .= join(" + ", @$coutargs);
    $text .= ");\n";
    $self->addtext($text);
    return $text;
}

sub do_postinc($$) {
    my ($self, $id, $op) = @_;
    my $text = "$id$op;\n";
    $self->addtext($text);
    return $text;
}

sub do_preinc($$) {
    my ($self, $arg1, $arg2) = @_;
    return $self->do_postinc($arg1, $arg2);
}

sub do_assignment($$) {
    my ($self, $assignee, $value) = @_;
    my $text = "$assignee = $value;\n";
    $self->addtext($text);
    return $text;
}

return 1;
