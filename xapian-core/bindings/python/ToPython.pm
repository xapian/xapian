# ToPython.pm: module to output Python code from the ad-hoc apitest parser.
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

package ToPython;

BEGIN {
    %ToPython::basic_types = (
    	"std::string" => 1,
    	"bool" => 1,
    	"om_weight" => 1,
	"int" => 1
    );

    $ToPython::int_type = '(?:(?:unsigned )?int|unsigned|size_t)';
}

sub new {
    my $class = shift;
    return bless { tests => "", func => ""}, $class;
}

sub get_value {
    my $self = shift;
    return $self->{func};
}

sub preamble {
    return <<PREAMBLE
import sys
import os
from omuscat import *
import apitest_helpers
from apitest_helpers import *

PREAMBLE
}

sub prologue{
    my $self = shift;
    my $result = <<MAINSTART;
verbose = 0
abort_on_error = 0
tests = []
for arg in sys.argv[1:]:
    if arg == "-v":
	verbose = 1
    elif arg == "-a":
        abort_on_error = 1
    else:
	tests.append(arg)
	tests.append("test_" + arg)

# a hack...
apitest_helpers.verbose = verbose

os.system("rm -fr .sleepycat/")

succeeded = 0;
failed = 0;
MAINSTART
    my $name;
    foreach $name (split(/ +/, $self->{tests})) {
        if ($name eq "" or $name eq "test_alwaysfail") { next};
        $result .= <<TESTEND;
if (len(tests) == 0) or ("$name" in tests):
    print "${name}... ",
    # another hack - should use a wrapper function or something
    if abort_on_error:
        result = ${name}()
	if result:
	    print "ok."
	    succeeded = succeeded+1
	else:
	    print "FAIL"
	    failed = failed+1
    else:  # !abort_on_error
      try:
        result = ${name}()
	if result:
	    print "ok."
	    succeeded = succeeded+1
	else:
	    print "FAIL"
	    failed = failed+1
      except (TestFail):
        print "FAIL"
	failed = failed+1
      except:
        print "EXCEPT"
	failed = failed+1
TESTEND
    }
    $result .= <<MAINEND
print succeeded, "tests passed, ", failed, "tests failed."
MAINEND
}

sub make_comment($) {
    my ($self, $text) = @_;

    return "# $text \n";
}

sub func_start($) {
    my $self = shift;
    my $orig = shift;
    if ($orig =~ /^bool ([a-z0-9_]+)\(\)/) {
        my $name = $1;
	$self->{func} = "def $name():\n";
	if (defined($self->{indent_level}) and $self->{indent_level} != 4) {
	    # if the last function wasn't finished properly, remove it from
	    # the list.
	    $self->{tests} =~ s/test_[a-z0-9]+$//;
	}
	$self->{tests} .= " $name";
	$self->{indent_level} = 8;
	return $self->{func};
    } else {
        Carp::croak("Don't understand function header");
    }
}

sub addtext($) {
    my ($self, $text) = @_;

    # do a few simple transformations to make it closer to java...
    #
    # remove address-of operator
    $text =~ s/\&([a-z]+)/$1/g;
    # change ||,&&,! to or,and,not
    $text =~ s/\|\|/or/g;
    $text =~ s/\&\&/and/g;
    $text =~ s/\!([^=])/not $1/g;
    # change true,false to 1,0
    $text =~ s/\btrue\b/1/g;
    $text =~ s/\bfalse\b/0/g;
    # change foo.size() into len(mymset.items)
    $text =~ s/($apitest_parser::func)\.size\(\)/len($1)/g;
    # change mset.items[foo].did to mset.items[foo][OMMSET_DID], etc.
    $text =~ s/\.did\b/[OMMSET_DID]/g;
    $text =~ s/(mset\.items\[[^\]]*])\.wt\b/${1}[OMMSET_WT]/g;
    $text =~ s/\.collapse_key\b/[OMMSET_COLLAPSE_KEY]/g;
    # change eset.items[foo].did to eset.items[foo][OMESET_DID], etc.
    $text =~ s/\.tname\b/[OMESET_TNAME]/g;
    $text =~ s/(eset\.items\[[^\]]*])\.wt\b/${1}[OMESET_WT]/g;
    # account for the renaming of some constructors
    $text =~ s/OmQuery\(\)/OmQueryNull()/g;
    while ($text =~ s/OmQuery\((OM_MOP[A-Z_]+),(.*)((?:, *[0-9]+)?)\)/OmQueryList($1, ($2)$3)/sg) {};
    # turn NULL into "NULL"
    $text =~ s/NULL/"NULL"/g;

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

sub parens_match($) {
    my $text = shift;

    my @parens = ();
    my @chars = split(/[^(){}[\]]*/, $text);
#    print STDERR "$text -> ", join(',', @chars), "\n";
    while (@chars) {
        my $char = shift @chars;
	if ($char =~ /[({[]/) {
	    push @parens, $char;
	} elsif ($char eq "}") {
	    return 0 if $#parens == -1;
	    return 0 if pop @parens ne "{";
	} elsif ($char eq "]") {
	    return 0 if $#parens == -1;
	    return 0 if pop @parens ne "[";
	} elsif ($char eq ")") {
	    return 1 if (join('', @parens) =~ /^\)*$/) and $#chars == -1;
	    return 0 if $#parens == -1;
	    return 0 if pop @parens ne "(";
	} elsif ($char eq "") {
	} else {
	    die "Internal error in parens_match(): got char `$char'";
	}
    }
    return (join('', @parens) =~ /^\)*$/);
}

sub var_decl($$$) {
    my ($self, $type, $name, $initialiser) = @_;
    my $text;
    if (defined $initialiser) {
        if ($initialiser =~ /^($apitest_parser::func)\((.*)\)$/) {
	    my ($func, $args) = ($1, $2);
	    if (&parens_match($args)) {
		$text = "$name = $func($args)\n";
	    } else {
	        # it's a parameter list
		$text = "$name = $type($func($args))\n"
	    }
	} else {
	    if (exists($ToPython::basic_types{$type})) {
		$text = "$name = $initialiser\n";
	    } else {
	        $text = "$name = $type($initialiser)\n"
	    }
	}
    } else {
        $text = "$name = $type()\n";
    }

    $self->addtext($text);
    return $text;
}

sub func_call($$) {
    my ($self, $func, $args) = @_;

    my $text = "$func($args)\n";
    $self->addtext($text);
    return $text;
}

sub do_if($) {
    my ($self, $cond) = @_;

    my $text = "if ($cond):\n";
    $self->addtext($text);
    $self->indent();
    return $text;
}

sub do_elsif($) {
    my ($self, $cond) = @_;

    $self->undent();
    my $text = "elif ($cond):\n";
    $self->addtext($text);
    $self->indent();
    return $text;
}

sub do_else() {
    my $self = shift;

    $self->undent();
    my $text = "else:\n";
    $self->addtext($text);
    $self->indent();
    return $text;
}

sub do_blank() {
    my $self = shift;
    my $text = "\n";
    # don't want the indenting here.
    $self->{func} .= $text;
    return $text;
}

sub do_comment($) {
    my ($self, $comment) = @_;
    my $text = $self->make_comment($comment);
    $self->addtext($text);
    return $text;
}

sub do_return($) {
    my ($self, $val) = @_;
    my $text = "return $val\n";
    $self->addtext($text);
    return $text;
}

sub close_block() {
    my $self = shift;
    my $text = "\n";
    $self->undent();
    $self->addtext($text);
    return $text;
}

sub do_cout(@$) {
    my ($self, $coutargs, $endl) = @_;
    my $text = "print str(";
    $text .= join(") + str(", @$coutargs);
    $text .= ")";
    if ($endl) {
        $text .= ",";
    }
    $text .= "\n";
    $self->addtext($text);
    return $text;
}

sub do_invalid($) {
    my ($self, $line) = @_;
    my $text = "#INVALID:$line";
    $self->addtext($text);
    die "Can't handle invalid line";
    return $text;
}

sub do_assignment($$) {
    my ($self, $assignee, $value) = @_;
    my $text = "$assignee = $value\n";
    $self->addtext($text);
    return $text;
}

sub do_try() {
    my $self = shift;
    my $text = "try:\n";
    $self->addtext($text);
    $self->indent();
    return $text;
}

sub do_catch($) {
    my ($self, $expt) = @_;

    # Extract the type
    if ($expt =~ /(?:const )?($apitest_parser::type )(?:\&)?(?$apitest_parser::identifier)?$/) {
        $expt = $1;
    }
    my $text = "except ($expt):\n";
    $self->undent();
    $self->addtext($text);
    $self->indent();
    return $text;
}

sub do_break() {
    my $self = shift;
    my $text = "break\n";
    $self->addtext($text);
    return $text;
}

sub do_for($$$) {
    my ($self, $precommand, $cond, $inc) = @_;
    my $text;

    # if of form "for (i=0; ...)"...
    if ($precommand =~ /^$ToPython::int_type ($apitest_parser::identifier) *= *([0-9]+)$/) {
        my ($indexvar, $initial) = ($1, $2);
        # and simple increment...
        if ($inc =~ /^($indexvar\+\+)|(\+\+$indexvar)$/) {
	    # and simple end test...
	    if ($cond =~ /^$indexvar *< *($apitest_parser::func\(\)|[0-9]+)$/) {
	        my $bound = $1;
	        $text = "for $indexvar in range($initial, $bound):\n";
	    } else {
		print STDERR "Bad cond in do_for($precommand, $cond, $inc)\n";
		die "Problem in do_for";
	    }
        } else {
            print STDERR "Bad increment in do_for($precommand, $cond, $inc)\n";
	    die "Problem in do_for";
	}
    } else {
        print STDERR "Bad precommand in do_for($precommand, $cond, $inc)\n";
	die "Problem in do_for";
    }
    
    $self->addtext($text);
    $self->indent();
    return $text;
}

sub do_postinc($$) {
    my ($self, $id, $op) = @_;
    my $text;
    if ($op eq '++') {
        $text = "$id = $id + 1\n";
    } else {
        $text = "$id = $id - 1\n";
    }
    $self->addtext($text);
    return $text;
}

sub do_preinc($$) {
    my ($self, $arg1, $arg2) = @_;
    return $self->do_postinc($arg1, $arg2);
}

return 1;
