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
$cvsmine = "./cvsminesearch";
$num_query = "100"; # to get 100 matches
$cvsquery = "./cvsquerydb";
# UNUSED $own = "./QueryCommit.cgi"; # name of this script


# control character separator
$ctrlA = chr(01);

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
if (param()) {
    $symbol = param("symbol");
    if (!$symbol) {
	&error("You must specify a symbol!");
    }
    @tmpsymbol = split /\s/, $symbol;
    $urlsymbol = join '|', @tmpsymbol;
    $query = param("query");
    my $root = Cvssearch::sanitise_root(param("root"), "root0");

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
	<tr><td colspan=2 class=s><FONT face=arial,sans-serif size=-1>
	<b>Commit Comments</b></FONT></td>
	<td class=s><FONT face=arial,sans-serif size=-1>
	<b>Package</b></FONT></td>
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
	#print "$cvsmine $root/db/mining.om $num_query $query $querysymbol";
	@ids = `$cvsmine $root/db/mining.om $num_query $query $querysymbol`;
	$gquery = shift @ids;
	if($gquery){
		@tmpquery = split /\s/, $gquery;
		$grepquery = join "|", @tmpquery;
		#print "$gquery & $grepquery";
	}
	#convert into local commit id
	@globalCommits = &Cvssearch::g2l_commit($CVSDATA, $root, @ids);
	
	for($i=0; $i <= $#globalCommits ; $i++){
		$pkg = $globalCommits[$i];
		$i++;
		$id = $globalCommits[$i];
		%pkgMAPid = &insertArray(\%pkgMAPid, $pkg, $id);
	}
	
	while (($key,$val)=each %pkgMAPid) {
		$querystr = "$cvsquery $root $key";
		$k=0;
		@info ={};
		foreach (@$val) {
			$info[$k] = "pkg=$key&id=$_";
			$querystr .= " -C $_";
			$k++;
		}
		$origcomments = `$querystr`;
		@comments = split /$ctrlA/, $origcomments;
		
		for($i=0;$i<$k;$i++){
        	if($comments[$i]){
        		$curcomments = Entities::encode_entities($comments[$i]);
	        	if($flag <0){
	        		print "<tr class=o";
	        	}else{
	        		print "<tr";
	        	}
	        	print " valign=top><td><pre><a href=\"./QueryComment.cgi?$info[$i]&symbol=$urlsymbol&root=$root\">Browse Code</a></pre></td>";
	        	if($query){
	        		$curcomments = &highlightquery($curcomments);
	        	}
	        	print "<td><pre>$curcomments</pre></td>";
	        	print "<td><pre>$key</pre></td>";
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
		$hash{$key} = [$value]; # anon array ref
	}
	return %hash;
}


#-----------------------------------
# highlightquery
# make words matched by query bold
#-----------------------------------
sub highlightquery{
	my ($words) = @_;
	$words =~ s!($grepquery)!<b>$1</b>!ig;
	return $words;
}
