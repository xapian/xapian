#-------------------------------------------------------------------
# some stuff about cvssearch
# This produces a html query interface for cvssearch
#
# Structure:
# 1. Calls $cvssearch - interface to Xapian - for query matches
# 2. grep source files for query words
# 3. sorts and rank query matches as per file
# 4. finds context matched by calling on $query
# 5. Displays results, linking them to QueryFile.cgi
#
# Author: Annie - anniec@cse.unsw.edu.au
# Date: Feb 16 2001
#-------------------------------------------------------------------

use strict;
use CGI ':all';
use Cvssearch;
use Entities;

#---------------------------------------------
# global mappigns defind in the script
#---------------------------------------------
my %lineMAPweight; # map line number with its weight
my $grepquery;

#-------------
# start html
#-------------

print "Content-type:text/html\n\n";
print "<html>\n";
print "<header>\n";
# ----------------------------------------
# common style sheet
# ----------------------------------------
Cvssearch::print_style_sheet();
print "</header>\n";
print "<body>\n";

#----------------------------------------
# Parse Parameters
#----------------------------------------
if(param()){
    my $dump = param("dump");
    my $id   = Cvssearch::decode(param("id"));
    my $found = Cvssearch::findfile($dump,$id);

    if (!$found){
        error("Page expired");
    }
    my @entries = split /\n/, $found;
	shift @entries;             #query
    shift @entries;             #stemquery
    $grepquery = shift @entries;
	shift @entries;             #id
	my $path = shift @entries;
	shift @entries;             #revs
	
    # ----------------------------------------
	# go through each line entry
    # ----------------------------------------
	foreach (@entries){
		my ($line, $weight) = split /[\s]+/, $_;
		$lineMAPweight{$line} = $weight;
	}

	print Cvssearch::fileheader("<b>Source Code<b>", "Darker highlight denotes better match");	
	
	print "<table cellSpacing=0 cellPadding=0 width=100% border=0>";
	my @file = `cat $path`;
	my $i=1;                       #line index
	foreach(@file){
		chomp;
		my $line = $_;
		$line = Entities::encode_entities($line);
		print "<tr nowrap><td><pre><a name=$i>$i</a></td>";

        # ----------------------------------------
        # create an extra character if the line
        # is empty
        # ----------------------------------------
        my $space ="";
        if (length($line) == 0) {
            $space = " ";
        }
		if($lineMAPweight{$i}) {
			my $weight = $lineMAPweight{$i};
			my $color = Cvssearch::get_color($weight, 150);
			$line = highlightquery($line);
			print "<td bgcolor=$color>";
		} else {
			print "<td>"
		}
		print "<pre>$line$space</td></tr>\n";
		$i++;
	}
	print "</table>";	
}
print "</body></html>";

#-----------------------------------
# highlightquery
# return line matched by query words
# make words matched by query bold
#-----------------------------------
sub highlightquery{
	my ($words) = @_;
	$words =~ s!($grepquery)!<b>$1</b>!ig;
	return $words;
}

#------------------
# display errors
#------------------
sub error{
	my ($mesg) = @_;
	print "<p><b class=red>$mesg</b>";
	exit(0);
}
