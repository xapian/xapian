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
$cvscommit = "./cvscommitsearch";
$cvsrelated = "./cvsrelatedsearch";
$commit_num_query = "10000"; # to get 100 matches
$related_num_query = "100";
$cvsquery = "./cvsquerydb";
# UNUSED: $own = "./CommitSearch.cgi"; # name of this script


# control character separator
$ctrlA = chr(01);

#-------------
# start html
#-------------

print "Content-type:text/html\n\n";
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
if(param()){
	$symbol = param("symbol");
	@tmpsymbol = split /\s/, $symbol;
	$urlsymbol = join '|', @tmpsymbol;
	$query = param("query");
	$root = param("root");
	



        if(!$root||$root eq "All"){ 
                 $root = "root0"; 
        }



	if($query){
		@tmpquery = split /\s/, $query;
		$grepquery = join "|", @tmpquery;
	}

	#----------------------------------------
	# print heading and beginning of table
	#----------------------------------------
	print "<table width=100%><tr><td><h1>Commits for <font color=black> $query $symbol</font></h1></td>";
	print "<td align=right><a href=\"./Query.cgi\" target=_top>Search Again</a></td></tr></table>";

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
	
	#query cvscommitsearch for global commit id
	#print "$cvscommit $root/db/commit.om $commit_num_query $query $querysymbol";

	  $query =~ s/;/\\;/g; # used in in: command
print STDERR "QUERY IS -$query-\n";
	    if ( $query =~ /@/ ) {
	      @ids = `$cvsrelated $root/db/related.om $related_num_query $query $querysymbol`;
	    } else {
	      @ids = `$cvscommit $root/db/commit.om $commit_num_query $query $querysymbol`;
            }
	$gquery = shift @ids;
	if($gquery){
		@tmpquery = split /\s/, $gquery;
		$grepquery = join "|", @tmpquery;
		#print "$gquery & $grepquery";
	}
	#convert into local commit id
	@globalCommits = &Cvssearch::g2l_commit($CVSDATA, $root, @ids);
	



	for($i=0; $i <= $#globalCommits ; $i++){
         
         $key = $globalCommits[$i];#pkg
         $i++;
         $val = $globalCommits[$i];#id
         # now that we have package and id, can print things out here

         # get comments  
         $origcomments = `$cvsquery $root $key -C $val`;
         @comments = split /$ctrlA/, $origcomments;

         if($comments[0]){ #hum.. might not even need this if with only one comment
                 $curcomments = Entities::encode_entities($comments[0]);
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
		$hash{$key} = [$value]; # anon list ref
	}
	return %hash;
}


#-----------------------------------
# highlightquery
# make words matched by query bold
#-----------------------------------
sub highlightquery{
	my ($words) = @_;

	  if ($query =~ /@/ ) {
	    # don't highlight for now
	  } else {

	    $words =~ s!(^|[^a-zA-Z]+)($grepquery)!$1<b>$2</b>!ig;
          }


	return $words;
}
