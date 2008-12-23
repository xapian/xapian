#! /usr/local/bin/perl  -w
#-------------------------------------------------------------------
# Displays commit comments for a particular symbol
#
# Author: Annie - anniec@cse.unsw.edu.au
# Date: May 9 2001
#-------------------------------------------------------------------

use CGI ':all';
use Cvssearch;
use Entities;

#-------------------
# path variables
#-------------------
$CVSDATA = &Cvssearch::get_cvsdata(); # path where database content file is stored
$cvsmine = "./cvscommitsearch";
$num_query = "10000"; # to get 100 matches
$cvsquery = "./cvsquerydb";
$own = "./CommitSearch.cgi"; # name of this script


# control character separator
$ctrlA = chr(01);
$ctrlB = chr(02);
$ctrlC = chr(03);

#-------------
# start html
#-------------

print "Content-type:text/html\n\n";
print "<html>\n";
print "<head>\n";
print "<title>CVSSearch : Commit Search</title>\n";

#---------------
# style sheet
#---------------
Cvssearch::print_style_sheet();
print "</head>\n";
print "<body>\n";


#----------------------------------------
# first pass on input
# to get query for printing out the form
#----------------------------------------
if(param()){
	$symbol = param("symbol");
	@tmpsymbol = split /\s/, $symbol;
	$urlsymbol = join '|', @tmpsymbol;
	$query = param("query");
	$root = param("root");
	
#	if(!$root){
#		&error("You must specify a root and symbol!");
#	}

	$root = "root0";

	if($query){
		@tmpquery = split /\s/, $query;
		$grepquery = join "|", @tmpquery;
	}

	#----------------------------------------
	# print heading and beginning of table
	#----------------------------------------
	print "<table width=100%><tr><td><h1>Commits for <font color=black> $query $symbol</font></h1></td>";
	print "<td align=right><a href=\"./Query.cgi\">Search Again</a></td></tr></table>";

	print <<_HTML_;
	<table cellSpacing=0 cellPadding=2 width="100%" border=0>
	<TBODY>
	<tr><td class=s><FONT face=arial,sans-serif size=-1>
	<b>Package</b></FONT></td>
	<td colspan=2 class=s><FONT face=arial,sans-serif size=-1>
	<b>Commit Comments</b></FONT></td>

	</tr>
_HTML_

	$flag=1;
	
	#------------------------------
	# query comments
	#------------------------------
	
	# parse symbol
	@symbols = split / /, $symbol;
	
	$querysymbol = ""; #format symbol into query format, i.e. add : in front of each
	foreach (@symbols) {
		$querysymbol .=	" \":$_\"";
	}
	
	#query cvsminesearch for global commit id
	#print "$cvsmine $root/db/commit.om $num_query $query $querysymbol";

	$query =~ s/;/\\;/g;
	@ids = `$cvsmine $root/db/commit.om $num_query $query $querysymbol`;
	$gquery = shift @ids;
	if($gquery){
		@tmpquery = split /\s/, $gquery;
		$grepquery = join "|", @tmpquery;
		#print "$gquery & $grepquery";
	}
	#convert into local commit id
	@globalCommits = &Cvssearch::g2l_commit($CVSDATA, $root, @ids);
	

        my @key_list = ();
	my @val_list = ();

	for($i=0; $i <= $#globalCommits ; $i++){
		$pkg = $globalCommits[$i];
		$i++;
		$id = $globalCommits[$i];
		%pkgMAPid = &insertArray(\%pkgMAPid, $pkg, $id);
		push @key_list, $pkg;
		push @val_list, $id;
	}
	
	my $key;
	my $val;
	my $ctr = -1;
# don't want to sort by package
	foreach $key (@key_list) { # key is package
		$ctr++;
                $val = $val_list[$ctr]; # get id for package
		$querystr = "$cvsquery $root $key";
		$k=0;
             	  @info ={};
		    $info[$k] = "pkg=$key&id=$val";
		      $querystr .= " -C $val";
		      $k++;

		$origcomments = `$querystr`;
		@comments = split /$ctrlA/, $origcomments;
		
		for($i=0;$i<$k;$i++){
        	if($comments[$i]){
        		$curcomments = Entities::encode_entities($comments[$i]);
	        	if($query){
	        		$curcomments = &highlightquery($curcomments);
	        	}
	        	if($flag <0){
	        		print "<tr class=o";
	        	}else{
	        		print "<tr";
	        	}
			print " valign=top>";
	        	print "<td><pre>$key</pre></td>";

	        	print "<td><pre><a href=\"./QueryComment.cgi?$info[$i]&symbol=$urlsymbol&root=$root\">Browse Code</a></pre></td>";

	        	print "<td><pre>$curcomments</pre></td>";

				print "</tr>";
				$flag *= -1;
			}
		}
	}

	print "</table>";	
	print "</body></html>";
}
	

#------------------
# display errors
#------------------
sub error{
	($mesg) = @_;
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
sub insertArray{
	my ($hashptr,$key,$value) = @_;
	my %hash = %$hashptr;
	if($hash{$key}){
		my $cur = $hash{$key};
		push @$cur, $value;
	}else{
		push my @cur, $value;
		$hash{$key} = \@cur;
	}
	return %hash;
}


#-----------------------------------
# highlightquery
# make words matched by query bold
#-----------------------------------
sub highlightquery{
	my ($words) = @_;
	$words =~ s/($grepquery)/<b>\1<\/b>/ig;
	return $words;
}
