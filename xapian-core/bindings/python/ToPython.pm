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
	"int" => 1
    );
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

TestFail = 'TestFail'
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

os.system("rm -fr .sleepy/")

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
    $text =~ s/\.wt\b/[OMMSET_WT]/g;
    $text =~ s/\.collapse_key\b/[OMMSET_COLLAPSE_KEY]/g;

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
    my $text;
    if (defined $initialiser) {
        if ($initialiser =~ /($apitest_parser::func)(\(.*\))$/) {
	    my ($func, $args) = ($1, $2);
	    $text = "$name = $func$args\n";
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

return 1;

__DATA__
sub do_elsif($) {
    my ($self, $cond) = @_;

    my $text = "} else if ($cond) {\n";
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

sub do_break() {
    my $self = shift;
    my $text = "break;\n";
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

