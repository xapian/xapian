# apitest_parser.pm - a quick-and-dirty parser for apitest.cc for convertion
#                     into other languages.
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
package apitest_parser;

use strict;
use Carp;

$apitest_parser::verbose = 0;

sub doprint(@) {
    if ($apitest_parser::verbose) {
        print @_;
    }
}

sub getline() {
    my $line = <STDIN>;
    chomp $line;
    if (defined $line) {
	# get a full statement if it looks like one without
	# curly braces.
	my $newbit;
	while ($line =~ /^[ \t]*[a-zA-Z]/ &&
	       $line !~ /[{};]/ &&
	       defined($newbit = <STDIN>)) {
	    chomp $newbit;
	    $line .= $newbit;
	}

        # remove leading/trailing whitespace
	$line =~ s/^[ \t]*(.*?)[ \t]*$/$1/;
    }
    return $line;
}

$apitest_parser::func_start_regex = '^bool (test_[a-z_]*[0-9]*)\(\) *$';
$apitest_parser::type = "\\b(?:bool|(?:unsigned )?int|unsigned|size_t|Om[A-Z][A-Za-z]+|om_[a-z]+|(?:std::)?string)";
$apitest_parser::identifier = "(?:\\b[a-zA-Z_0-9]+\\b)";
$apitest_parser::func = "(?:(?:$apitest_parser::identifier\\.)*$apitest_parser::identifier)";
$apitest_parser::commentstart = '(?:\/\/|\/\*)';

sub new($) {
    my ($class, $interp) = @_;
    my $self = {};
    $self->{interp} = $interp;
    die "Can't create interpreter!" unless defined $self->{interp};
    return bless $self, $class;
}

sub make_comment($) {
    my ($self, $comment) = @_;
    return $self->{interp}->make_comment($comment);
}

sub preamble() {
    my $self = shift;
    return $self->{interp}->preamble();
}

sub prologue() {
    my $self = shift;
    return $self->{interp}->prologue();
}

sub parse_function($) {
    my ($self, $firstline) = @_;
    $self->{failed} = 0;

    doprint "Found test: $firstline\n";
    $self->{interp}->func_start($firstline);
    $self->{indent} = 4;
    #print "Converted to: " . 

    if (getline() ne "{") {
	Carp::croak "Expected opening block!";
    }
    while (defined ($_ = getline())) {
	my $line = $_;
	my $newline = $self->parse_line($line);
	last if (defined $newline and $newline eq "END"); 
	if ($main::verbose) {
	    $newline = (' ' x $self->{indent}) . $newline;
	    $newline =~ s/^( *)-=>/-=>$1/;
	    print $newline;
	}
    }
    if ($self->{failed}) {
        return undef;
    }
    return $self->{interp}->get_value();
}

sub parse_line($) {
    my ($self, $line) = @_;
    my $interp = $self->{interp};
    my $newline;
    if ($line =~ /^($apitest_parser::type) ($apitest_parser::identifier)( = (.*))?;/) {
	my ($type, $id, $init) = ($1, $2, $4);
	$newline = $interp->var_decl($type, $id, $init);
    } elsif ($line =~ /^($apitest_parser::type) ($apitest_parser::identifier)\((.*)\);/) {
	my ($type, $id, $init) = ($1, $2, $3);
	$newline = $interp->var_decl($type, $id, $init);
    } elsif ($line =~ /^($apitest_parser::func)\((.*)\);/) {
	my ($func, $args) = ($1, $2);
	$newline = $interp->func_call($func, $args);
    } elsif ($line =~ /^if *\((.*)\) {$/) {
	my $cond = $1;
	$newline = $interp->do_if($cond);
	$self->{indent} += 4;
    } elsif ($line =~ /^if *\(.*/) {
	while ($line !~ /^if *\(.*\) .*[;{]/s) {
	    my $extra = getline();
	    if (!defined $extra) {
	        print "-=>$line";
		die "End of file in if statement";
	    }
	    $line .= $extra;
	}
	if ($line =~ /if *\((.*)\) {$/s) {
	    my $cond = $1;
	    $newline = $interp->do_if($cond);
	    $self->{indent} += 4;
	} elsif ($line =~ /if *\((.*)\) ([^;]*);$/s) {
	    my ($cond, $statement) = ($1, $2);
	    #print "cond = \"$cond\", statement = \"$statement\"\n";
	    $newline = $interp->do_if($cond);
	    $self->{indent} += 4;
	    $newline .= ' ' x $self->{indent};
	    $newline .= $self->parse_line("$statement;", $interp);
	    $self->{indent} -= 4;
	    $newline .= ' ' x $self->{indent};
	    $newline .= $interp->close_block();
	} else {
	    print "-=>$line";
	    die "Invalid if statement";
	}
    } elsif ($line =~ /^for \( *(.*); *(.*); *(.*)\) {$/) {
	my ($precommand, $cond, $inc) = ($1, $2, $3);
	$newline = $interp->do_for($precommand, $cond, $inc);
	$self->{indent} += 4;
    } elsif ($line =~ /^try {$/) {
        $newline = $interp->do_try();
	$self->{indent} += 4;
    } elsif ($line =~ /^} catch ?\((.*)\) {$/) {
        my $expt = $1;
        $newline = $interp->do_catch($expt);
    } elsif ($line =~ /^{/) {
	$self->{indent} -= 4;
    } elsif ($line =~ /^($apitest_parser::commentstart)(.*)/) {
	my $commenttext = $2;
	$newline = $interp->do_comment($commenttext);
    } elsif ($line =~ /^} else {$/) {
        $newline = $interp->do_else();
    } elsif ($line =~ /^} else if \(/) {
	while ($line !~ /^} else if \(.*\) .*[;{]/s) {
	    my $extra = getline();
	    if (!defined $extra) {
	        print "-=>$line";
		die "End of file in if statement";
	    }
	    $line .= $extra;
	}
	if ($line =~ /} else if \((.*)\) {$/s) {
	    my $cond = $1;
	    $newline = $interp->do_elsif($cond);
	} elsif ($line =~ /} else if \((.*)\) ([^;]*);$/s) {
	    my ($cond, $statement) = ($1, $2);
	    #print "cond = \"$cond\", statement = \"$statement\"\n";
	    $newline = $interp->do_elsif($cond);
	    $self->{indent} += 4;
	    $newline .= ' ' x $self->{indent};
	    $newline .= $self->parse_line("$statement;", $interp);
	    $self->{indent} -= 4;
	    $newline .= ' ' x $self->{indent};
	    $newline .= $interp->close_block();
	} else {
	    print "-=>$line";
	    die "Invalid else if statement";
	}
    } elsif ($line =~ /^}$/) {
	$self->{indent} -= 4;
	$newline = $interp->close_block();
	if ($self->{indent} == 0) {
		doprint "End of function.\n";
	    return "END";
	}
    } elsif ($line =~ /^return (.*);/) {
# return statement
	my $returnval = $1;
	$newline = $interp->do_return($returnval);
    } elsif ($line =~ /^break;/) {
# break statement
	$newline = $interp->do_break();
    } elsif ($line =~ /^($apitest_parser::identifier) = (.*);/) {
# assignment
	my ($var, $value) = ($1, $2);
	$newline = $interp->do_assignment($var, $value);
    } elsif ($line =~ /^cout/) {
# uses of cout are often multiline, so group them together.
	while ($line !~ /\;/) {
	    $line .= getline();
	}
	if ($line !~ /^cout(.*?)(<< endl)?;/s) {
	    print STDERR "bad cout line: $line";
	    Carp::croak "Bad cout line!";
	}
	my ($coutargs, $endl) = ($1, $2);
	my @coutargs = split(/[ \t]*<<[ \t]*/s, $coutargs);
	shift @coutargs; # remove blank before first <<
		$newline = $interp->do_cout(\@coutargs, $endl);
    } elsif ($line =~ /^$/) {
	$newline = $interp->do_blank();
    } elsif ($line =~ /^($apitest_parser::identifier)(\+\+|\-\-);/) {
	my ($id, $op) = ($1, $2);
	$newline = $interp->do_postinc($id, $op);
    } elsif ($line =~ /^(\+\+|\-\-)($apitest_parser::identifier);/) {
	my ($id, $op) = ($1, $2);
	$newline = $interp->do_preinc($id, $op);
    } else {
	if ($main::verbose) {
	    $newline = "-=> $line\n";
	    $self->{failed} = 1;
	} else {
	    $newline = $interp->do_invalid($line);
	}
#	    print "# " . $line . "\n";
    }
    return $newline;
}
