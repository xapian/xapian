#-------------------------------------------------------------------
# some stuff about cvssearch
# This produces a html query interface for cvssearch
#
# Structure:
# 1. Calls $cvssearch - interface to omsee - for query matches
# 2. grep source files for query words
# 3. sorts and rank query matches as per file
# 4. finds context matched by calling on $query
# 5. Displays results, linking them to QueryFile.cgi
#
# Author: Annie - anniec@cse.unsw.edu.au
# Date: Feb 16 2001
#-------------------------------------------------------------------


use CGI ':all';
use Cvssearch;
use Entities;

#---------------------------------------------
# global mappigns defind in the script
#---------------------------------------------
#%lineMAPweight - map line number with its weight

#-------------
# start html
#-------------

print "Content-type:text/html\n\n";
print "<html>\n";
print "<header>\n";

#---------------
# style sheet
#---------------
print <<_STYLE_;
<STYLE TYPE=text/css>
<!--
body {background-color:white}
.red {color:red}
-->
</STYLE>
</header>
<body>
_STYLE_

#----------------------------------------
# Parse Parameters
#----------------------------------------
if(param()){
	$dump = param("dump");
	$id = param("id");
	$id = Cvssearch::decode($id);
	
	$found = Cvssearch::findfile($dump,$id);
	if (!$found){
		&error("Page expired");
	}
	@entries = split /\n/, $found;
	shift @entries;#query
	shift @entries;#stemquery
	$grepquery = shift @entries;
	shift @entries; #id
	$path = shift @entries;
	shift @entries; #revs
	
	#go through each line entry
	foreach (@entries){
		($line, $weight) = split / /, $_;
		$lineMAPweight{$line} = $weight;
	}
	
	#display file
	
	#filename
	&filename();
	
	print "<table cellSpacing=0 cellPadding=0 width=100% border=0>";
	@file = `cat $path`;
	#print @file;
	$i=1; #line index
	foreach(@file){
		s/\n//g;
		$line = $_;
		$line = Entities::encode_entities($line);
		print "<tr>";
		print "<td><pre><a name=$i>$i</a></td>";
        my $space ="";
        if (length($line) == 0) {
            $space = " ";
        }
		if($lineMAPweight{$i}){
			$weight = $lineMAPweight{$i};
			$color = Cvssearch::get_color($weight, 150);
			$line = &highlightquery($line);
			print "<td bgcolor=$color><pre>$line$space</td>";
		}else{
			print "<td><pre>$line$space</td>";
		}
		print "</tr>\n";
		$i++;
	}
	print "</table>";	
	
	#print an empty page so html can scroll to the last line
#	print "<pre>";
#	for($j=0;$j<60;$j++){
#		print "\n";	
#	}
#	print "</pre>";
}

print "</body></html>";


#--------------------------
# sub functions
#--------------------------

#!!!!!!!!!!!!!!!!!!!!
# HTML FORMATTING
#!!!!!!!!!!!!!!!!!!!!

#-----------------------------------
# stats prints stemmed query words
# projects searched in and
# number of files matched
#-----------------------------------

sub filename{
	print Cvssearch::fileheader("<b>Source Code<b>", "Darker highlight denotes better match");
}



#-----------------------------------
# highlightquery
# return line matched by query words
# make words matched by query bold
#-----------------------------------
sub highlightquery{
	my ($words) = @_;
	$words =~ s/($grepquery)/<b>\1<\/b>/ig;
	return $words;
}

#------------------
# display errors
#------------------
sub error{
	($mesg) = @_;
	&stats;
	print "<p><b class=red>$mesg</b>";
	exit(0);
}
