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
use cvssearch;
use Entities;

#-----------------
# paths
#-----------------
$source = "./Source.cgi";
$cvsquery = "./cvsquerydb";

#---------------------------------------------
# global mappigns defind in the script
#---------------------------------------------
#%lineMAPinfo - map line number with its info (weight grep rev1 rev2..)

# control character separator
$ctrlA = chr(01);

#-------------
# start html
#-------------

print "Content-type:text/html\n\n";
print "<html>\n";
print "<head>\n";

#print javascript for popups
open (OUTPUT, "<./popup.js");
print <OUTPUT>;
close (OUTPUT);

#---------------
# style sheet
#---------------
print <<_STYLE_;

<STYLE TYPE=text/css>
body {background-color:white}
A:link    { text-decoration:none; color:blue }
A:active  { text-decoration:underline; color:blue}
A:visited { text-decoration:none; color:blue }
.popupLink { COLOR: blue; outline: none }
.popup { POSITION:absolute; VISIBILITY:hidden; BACKGROUND-COLOR:white; LAYER-BACKGROUND-COLOR:white; BORDER:2px solid orange; PADDING: 3px; z-index: 10 }
.red {color:red}
</STYLE>

</head>
<body>
_STYLE_

#----------------------------------------
# parse parameters
#----------------------------------------
if(param()){
	$dump = param("dump");
	$id = param("id");
	$displayname = param("displayname");
	$did = cvssearch::encode($id);
	$ddisplayname = cvssearch::encode($displayname);
	$passparam = "?id=$did&dump=$dump&displayname=$ddisplayname";
	
	$id = cvssearch::decode($id);
	$displayname = cvssearch::decode($displayname);
	
	
	$found = cvssearch::findfile($dump,$id);
	if (!$found){
		&error("Page expired");
	}
	@entries = split /\n/, $found;
	$query = shift @entries;#query
	$stemquery = shift @entries;#stemquery
	$grepquery = shift @entries;
	shift @entries; #id
	$path = shift @entries;
	$revs = shift @entries; #revs
	
	#go through each line entry
	foreach (@entries){
		@info = split / |\n/, $_;
		$line = shift @info;
		$together = join ' ', @info;
		$lineMAPinfo{$line} = $together;
	}
	#-------------------------------
	# query cvsquery for comments
	#-------------------------------
	
	($root, $db, $fileid) = split / /, $id;
	$querystr = "$cvsquery $root $db";
	@revs = split /\s/, $revs;
	foreach(@revs){
		$querystr .= " -c $fileid $_";
	}
	
	$result = `$querystr`;
	@comments = split /$ctrlA/, $result; # split revisions
	
	#---------------------------
	#write comments into divs
	#---------------------------
	
	print "<!-- keep the popup divs as the first things on the page or else MSIE 5 on the mac sometimes has trouble rendering them on top of text -->\n";
	print "<DIV onclick='event.cancelBubble = true;' class=popup id='grep'>grep matched $grepquery on this line.</DIV>\n";
	$i=0;
	foreach (@revs){
		$curcomment = $comments[$i];
		$curcomment = Entities::encode_entities($curcomment);
		$curcomment = &highlightquery($curcomment);
		$ch = &toChar($_); # need to convert digits to alphabets since netscape doesn't understand digit id
		print "<DIV onclick='event.cancelBubble = true;' class=popup id='$ch'>$curcomment</DIV>\n";
		$i++;
	}
	print "<!-- begin body of document -->\n";

	#----------------
	#display file
	#----------------
	
	#filename
	&filename($stemquery);
	
	print "<table cellSpacing=0 cellPadding=0 width=100% border=0>";
	@file = `cat $path`;
	#print @file;
	$i=1; #line index
	foreach(@file){
		s/\n//g;
		$line = $_;
		if($lineMAPinfo{$i}){
			$line = Entities::encode_entities($line);
			print "<tr>";
			print "<td><pre>$i:</td>";
			$info = $lineMAPinfo{$i};
			@info = split / /, $info;
			$weight = shift @info;
			
			#print revs
			print "<td><pre>";
			foreach (@info){
				$currev = $_;
				$ch = &toChar($_); # need to convert digits to alphabets since netscape doesn't understand digit id
				print "<a href=# class=popupLink\n";
				print "onclick=\"return !showPopup('$ch', event);\"\n";
				print "onmouseover=\"return !showPopup('$ch', event);\"\n";
				print "onmouseout=\"hideCurrentPopup(); return false;\">$currev</a> ";
			}

			print "</td>";
			
			$color = cvssearch::get_color($weight, 150);
			$line = &highlightquery($line);
			print "<td bgcolor=$color><pre><a href=$source$passparam#$i target=source>$line</a></td>";
		}
		print "</tr>\n";
		$i++;
	}
	print "</table>";
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
	my ($name) = @_;
	$num = scalar(keys %lineMAPweight);
print <<_HTML_;
<p>
<TABLE cellSpacing=0 cellPadding=2 width="100%" border=0>
<TBODY>
<TR>
<TD noWrap bgColor=#3366cc><FONT face=arial,sans-serif color=white 
size=-1>Matched lines for <b>$name</b>&nbsp; </FONT></TD>
<TD noWrap align=right bgColor=#3366cc><FONT face=arial,sans-serif color=white 
size=-1>Darker highlight denotes better matches.</FONT></TD></TR></TBODY></TABLE>
_HTML_
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

#--------------------------------
# replace digits with characters
# 0->A 1->B 2->C ..etc
#--------------------------------
sub toChar{
	my ($word) = @_;
	$word =~ s/(\d)/@{[chr($1+65)]}/ig;
	return $word;
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
