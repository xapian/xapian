#!/usr/local/bin/perl
# makedepend.pl ---
#      Make dependency rule for make from C compiler output.
#                               Copyright (C) 1999, MIYASHITA Hisashi.
#
# This file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

# Comment:
#   This file is perl script that is executable on Perl version 4 or 5.
#   This program extract dependency information from output of GCC or
#   C preprocessor that can print out '#line' information.  Then format
#   and print out the information into the expected file.

#$makedepend_start = "### automatically generated dependency start\n";
$makedepend_start = "# DO NOT DELETE THIS LINE -- make depend depends on it.\n";
#$makedepend_end = "### automatically generated dependency end\n";
$makedepend_end = "hopefullythislinewon'toccurinthefile";
$usage = "makedepend.pl (-f|-F) OUTPUTFILE [-E] [-D] [-DO] [-ra] [[-rm REMOVE-RULE]  [-ag AGREEMENT-RULE] [-rp REPLACE-RULE] ...] [-cc COMPILER (msvc or gcc)] TARGETS...";
$remove_absolute_path = 0;
$generate_from_preprocess = 0;
$filetype_dos = 0;
$file_output_dos = 0;
$output_replace = 1;
$interpret_pattern = '^\#line[ \t]*(\d+)[ \t]+(\"[^\"]+\")$';

sub create_dep_gcc {
    local ($file) = @_;

    @out = ();
    open(DEPFILE, $file);
    while(<DEPFILE>) {
	chop;
	@sepline = split(/[\t ]/);

	for ($j = 0;$j <= $#sepline;$j++) {
	    $str = $sepline[$j];
	    if (($str !~ /^[\t ]*\\[\t ]*$/)
		&& ($str =~ /[^ \t]/)) {
		$str =~ s/:[ \t]*$//; # remove tail `:'
		if ($str =~ /[^ \t]/) {
		    push(@out, $str);
		}
	    }
	}
    }
    close(DEPFILE);

    return @out;
}

sub create_dep_cpp {
    local ($file) = @_;
    local ($srcfile, $line);

    @out = ();

    if ($filetype_dos) {
	$file =~ tr!\\!/!;
    }
    push (@out, $file);
    open(DEPFILE, $file);
    while(<DEPFILE>) {
	s/\r\n$/\n/;
	chop;
	if (/$interpret_pattern/) {
	    $line = $1;
	    $srcfile = $2;

	    if ($srcfile !~ /^\"<(built-in|command line)>\"$/) {

		if ($srcfile =~ /\"[^\"]+\"$/) {
		    eval '$srcfile = '.$srcfile;
		}

		if ($filetype_dos) {
		    $srcfile =~ tr!\\!/!;
		}

		@list = grep(($srcfile eq $_), @out);
		if (!($#list >= 0)
		    && ($srcfile =~ /[^ \t]/)) {
		    push(@out, $srcfile);
		}
	    }
	}
    }
    close(DEPFILE);

    return @out;
}

sub check_path {
    local ($file) = @_;
    local ($pat, $repl, $action);

    if ($remove_absolute_path) {
	if ($filetype_dos) {
	    if ($elem =~/^(\w:)?\//) {
		return "";
	    }
	}else{
	    if ($elem =~/^\//) {
		return "";
	    }
	}
    }
    for ($k = 0;$k <= $#pathsep_rules;) {
	$action = $pathsep_rules[$k++];
	if ($action eq "REMOVE") {
	    $pat = $pathsep_rules[$k++];
	    print "$elem <RM- $pat\n";
	    return "" if ($elem =~ $pat);
	} elsif ($action eq "REPLACE") {
	    $pat = $pathsep_rules[$k++];
	    $repl = $pathsep_rules[$k++];
	    eval "\$elem =~ s!$pat!$repl!";
	    print "$elem <RP- $pat + $repl\n";
	} elsif ($action eq "AGREEMENT") {
	    $pat = $pathsep_rules[$k++];
	    print "$elem <AG- $pat\n";
	    return $elem if ($elem =~ $pat);
	} else {
	    die("Internal error-I don't know such action:", $action);
	}
    }
    return $elem;
}

sub output_dependency {
    local ($file, $createdep, $replacep) = @_;
    local ($skip_to_end, $output);

    if ($replacep) {
	$skip_to_end = 0;
	$output = "";
	open(INPUTFILE, $file);
	while(<INPUTFILE>) {
	    s/\r\n$/\n/;
	    if ($_ eq $makedepend_start) {
		$output =  $output.$_.$createdep;
		$skip_to_end = 1;
	    }elsif ($_ eq $makedepend_end){
		$output =  $output.$_;
		$skip_to_end = 0;
	    }elsif ($skip_to_end == 0){
		$output =  $output.$_;
	    }
	}
	close(INPUTFILE);
    } else {
	$output = $createdep;
    }
    open(OUTPUTFILE, ">$file");
    print OUTPUTFILE $output;
    close(OUTPUTFILE);
}

##
## The end of function definition part---------------------------------------
##

for($i = 0;$i <= $#ARGV;$i++){
    if ($ARGV[$i] eq "-f"){
	$output_filename = $ARGV[++$i];
	$output_replace = 1;
    }elsif ($ARGV[$i] eq "-F"){
	$output_filename = $ARGV[++$i];
	$output_replace = 0;
    }elsif ($ARGV[$i] eq "-E"){
	$generate_from_preprocess = 1;
    }elsif ($ARGV[$i] eq "-D"){
	$filetype_dos = 1;
    }elsif ($ARGV[$i] eq "-DO"){
	$file_output_dos = 1;
    }elsif ($ARGV[$i] eq "-rp") {
	split(/=/, $ARGV[++$i]);
	if ($#_ != 1) {
	    die("-rp argument format must be ...=...:", $ARGV[$i]);
	}
	push(@pathsep_rules, "REPLACE");
	push(@pathsep_rules, @_);
    }elsif ($ARGV[$i] eq "-rm") {
	push(@pathsep_rules, ("REMOVE", $ARGV[++$i]));
    }elsif ($ARGV[$i] eq "-ag") {
	push(@pathsep_rules, ("AGREEMENT", $ARGV[++$i]));
    }elsif ($ARGV[$i] eq "-ra") {
	$remove_absolute_path = 1;
    }elsif ($ARGV[$i] eq "-cc") {
	if ($ARGV[++$i] eq "gcc") {
	    $interpret_pattern = '^\#[ \t]+(\d+)[ \t]+(\"[^\"]+\") 1$';
	} else {
	    $interpret_pattern = '^\#line[ \t]*(\d+)[ \t]+(\"[^\"]+\")$';
	}
    }else{
	push(@depfiles, $ARGV[$i]);
    }
}

if ($output_filename eq "") {
    die "Specify output filename!";
}

$createdep = "\n";

for ($i = 0;$i <= $#depfiles;$i++){

    if ($generate_from_preprocess) {
	@deplist = &create_dep_cpp($depfiles[$i]);
    } else {
	@deplist = &create_dep_gcc($depfiles[$i]);
    }

    for ($j = 0;$j <= $#deplist;$j++) {
	$elem = $deplist[$j];

	$elem = &check_path($elem);
	next if ($elem eq "");

	if ($file_output_dos) {
	    $elem =~ tr!/!\\!;
	}
	if ($j == 0) {
	    $createdep = $createdep.$elem.':';
	} else {
	    $createdep = $createdep." \\\n"."    ".$elem;
	}
    }
    $createdep = $createdep."\n\n";
}

&output_dependency($output_filename, $createdep, $output_replace);
