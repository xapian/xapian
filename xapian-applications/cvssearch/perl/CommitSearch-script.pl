#-------------------------------------------------------------------
# Displays commit comments for a particular symbol
#
# Author: Annie - anniec@cse.unsw.edu.au
# Date: May 9 2001
#-------------------------------------------------------------------

use strict;
use CGI ':all';
use Cvssearch;
use Entities;

#-------------------
# path variables
#-------------------
# path where database content file is stored
my $CVSDATA = &Cvssearch::get_cvsdata();
my $cvscommit = "./cvscommitsearch";
my $cvsrelated = "./cvsrelatedsearch";
my $commit_num_query = "10000"; # to get 100 matches
my $related_num_query = "100";
my $cvsquery = "./cvsquerydb";
# UNUSED: $own = "./CommitSearch.cgi"; # name of this script

# control character separator
my $ctrlA = chr(01);

#-------------
# start html
#-------------

print "Content-Type: text/html\n\n";
print "<html>\n";
print "<head>\n";

#---------------
# style sheet
#---------------
print <<_HTML_;
<style type=\"text/css\">
body {background-color:white;}
h1 {color:#0066cc; font-size:large}
.s {background-color:#3366CC; color:#FFFFFF;}
.o {background-color:#ccccee;}
.ref {color:#666666;font-size:12;font-family:"Arial, Helvetica, sans-serif"}

.popupLink { color: blue; outline: none;}
.popup { position:absolute; visibility:hidden; color:white;background-color:#3366cc}
layer-background-color:#3366cc;border:2px solid orange; padding: 3px; z-index: 10;}
</style>
</head>
<body>
_HTML_

#----------------------------------------
# first pass on input
# to get query for printing out the form
#----------------------------------------
my ($query, $grepquery);
if (param()) {
    my $symbol = param("symbol");
    my $urlsymbol = $symbol;
    $urlsymbol =~ s/\s+/|/g;

    $query = param("query");
    my $root = Cvssearch::sanitise_root(param("root"), "root0");

    if ($query) {
	$grepquery = $query;
	$grepquery =~ s/\s+/|/g;
    }

    #----------------------------------------
    # print heading and beginning of table
    #----------------------------------------
    print "<table width=100%><tr><td><h1>Commits for <font color=black> $query $symbol</font></h1></td>";
    print "<td align=right><a href=\"./Query.cgi\" target=_top>Search Again</a></td></tr></table>";

    print <<_HTML_;
	<table cellSpacing=0 cellPadding=2 width="100%" border=0>
	<tr><td class=s><FONT face=arial,sans-serif size=-1>
	<b>Package</b></FONT></td>
	<td colspan=2 class=s><FONT face=arial,sans-serif size=-1>
	<b>Commit Comments</b></FONT></td>

	</tr>
_HTML_

    my $flag = 1; # for alternating row style
	
    #------------------------------
    # query comments
    #------------------------------
    
    # format symbol into query format, i.e. add : in front of each
    my $querysymbol = "";
    foreach (split / /, $symbol) {
	$querysymbol .= " \":$_\"";
    }
    
    my @ids;
    $query =~ s/;/\\;/g; # used in in: command
#print STDERR "QUERY IS -$query-\n";
    if ($query =~ /\@/) {
	@ids = `$cvsrelated $root/db/related.om $related_num_query $query $querysymbol`;
    } else {
	# query cvscommitsearch for global commit id
	@ids = `$cvscommit $root/db/commit.om $commit_num_query $query $querysymbol`;
    }

    my $gquery = shift @ids;
    if ($gquery) {
	$grepquery = $gquery;
	$grepquery =~ s/\s+/|/g;
    }

    # convert into local commit id
    my @globalCommits = &Cvssearch::g2l_commit($CVSDATA, $root, @ids);

    for (my $i = 0; $i <= $#globalCommits; $i++) {
	my $key = $globalCommits[$i];#pkg
	$i++;
	my $val = $globalCommits[$i];#id
	# now that we have package and id, can print things out here

	# get comments
	#print "<!-- $cvsquery $root $key -C $val -->\n";
	my @comments = split /$ctrlA/o, `$cvsquery $root $key -C $val`;

	if ($comments[0]) {
	    #hum.. might not even need this if with only one comment
	    my $curcomments = Entities::encode_entities($comments[0]);
	    if ($query) {
		$curcomments = &highlightquery($curcomments);
	    } 
	    if ($flag < 0) { 
		print "<tr class=o";
	    } else { 
		print "<tr";
	    } 
	    print " valign=top>";
	    print "<td><pre>$key</pre></td>";

	    print "<td><pre>[<a href=\"./QueryComment.cgi?pkg=$key&id=$val&symbol=$urlsymbol&root=$root\" target=_top>code</a>/<a href=\"./CommitSearch.cgi?query=$key\@$val&root=$root\">similar</a>]</pre></td>";

	    print "<td><pre>$curcomments</pre></td>";

	    print "</tr>";
	    $flag *= -1;
	}
    }

    print "</table>";
    print "</body></html>";
}

#------------------
# display errors
#------------------
sub error {
    my ($mesg) = @_;
    print "<p><b class=red>$mesg</b></body></html>";
    exit(0);
}

#!!!!!!!!!!!!!!!!!!!!!!!
# Hashtable functions
#!!!!!!!!!!!!!!!!!!!!!!!

#--------------------------------------------
# Given a hashtable with array as its value
# push new value into array if entry exists
# Usage:
# %hash = &insertArray(\%hash, $key, $value);
#--------------------------------------------
sub insertArray {
    my ($hashptr, $key, $value) = @_;
    my %hash = %$hashptr;
    if ($hash{$key}) {
	my $cur = $hash{$key};
	push @$cur, $value;
    } else {
	$hash{$key} = [$value]; # anon list ref
    }
    return %hash;
}

#-----------------------------------
# highlightquery
# make words matched by query bold
#-----------------------------------
sub highlightquery {
    my ($words) = @_;

    if ($query =~ /@/ ) {
	# don't highlight for now
    } else {
	$words =~ s!(^|[^a-zA-Z]+)($grepquery)!$1<b>$2</b>!ig;
    }

    return $words;
}
