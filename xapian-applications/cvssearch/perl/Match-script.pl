#-------------------------------------------------------------------
# some stuff about cvssearch
# This produces a html with matched lines only and it's revision 
# information.
#
# Features include comments popups on mouseover, stick/unstick on click,
# color coding, separate column for each revision,
# bold revisions with query matches, bold grep matches
#
# Author: Annie - anniec@cse.unsw.edu.au
# Date: Feb 16 2001
#-------------------------------------------------------------------


use CGI ':all';
use Cvssearch;
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
#%revMAPcolor - maps revision with color coding
#%revMAPmatch - only stores value for matched revisions

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
A:link, A:active, A:visited { text-decoration:none;color:black;}
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
	$did = Cvssearch::encode($id);
	$ddisplayname = Cvssearch::encode($displayname);
	$passparam = "?id=$did&dump=$dump&displayname=$ddisplayname";
	
	$id = Cvssearch::decode($id);
	$displayname = Cvssearch::decode($displayname);
	
	
	$found = Cvssearch::findfile($dump,$id);
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
		@info = split /\s/, $_;
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
	print "<DIV onclick='event.cancelBubble = true;' class=popup id='grep'>grep matched <b>$grepquery</b> on this line.</DIV>\n";
	$i=0;
	foreach (@revs){
		$currev = $_;
		$curcomment = $comments[$i];
		chomp $curcomment;
		$curcomment = Entities::encode_entities($curcomment);
		if($curcomment =~/$grepquery/){
			$revMAPmatch{$currev} = $currev;
			$curcomment = &highlightquery($curcomment);
		}
		$ch = &toChar($currev); # need to convert digits to alphabets since netscape doesn't understand digit id
		print "<DIV onclick='event.cancelBubble = true;' class=popup id='$ch'><pre><b>Rev:$currev</b>\n$curcomment</pre></DIV>\n";
		$i++;
	}
	print "<!-- begin body of document -->\n";

	#----------------
	#display file
	#----------------
	
	#filename
	&filename($stemquery);
	@colors = Cvssearch::getSpectrum($#revs+1);
	print "<pre>";
	foreach ($i=0;$i<$#revs+1;$i++){
		$revMAPcolor{$revs[$i]} = $colors[$i];
		$ch = &toChar($revs[$i]); # need to convert digits to alphabets since netscape doesn't understand digit id
		print "<span style=\"background-color:$colors[$i]\">";
		print "<a href=# \n";
		print "onclick=\"locking('$ch', event);return false;\"\n";
		print "onmouseover=\"return !showPopup('$ch', event);\"\n";
		print "onmouseout=\"hideCurrentPopup(); return false;\"\n";
		print ">";
		if($revMAPmatch{$revs[$i]}){
			print "<b>$revs[$i]</b>";
		}else{
			print "$revs[$i]";
		}
		print "</a></span> ";
	}
	print "</pre>\n";
	
	print "<table cellSpacing=0 cellPadding=0 width=100% border=0>";
	@file = `cat $path`;
	#print @file;
	$i=1; #line index
	push @revs, "grep";
	foreach(@file){
		s/\n//g;
		$line = $_;
		if($lineMAPinfo{$i}){
			$line = Entities::encode_entities($line);
			print "<tr>";
			print "<td><a name=$i><pre>$i:</td>";
			$info = $lineMAPinfo{$i};
			@info = split / /, $info;
			$weight = shift @info;
			
			#print revs, in the column it belongs to
			print "<td><pre>";
			$flag = 1;
			for($j=0;$j<$#revs+1;$j++){
				$toprev = $revs[$j];
				$found = 0;
				$flag *= -1;
				foreach (@info){
					if($toprev eq $_){
						$currev = $_;
						$color = $revMAPcolor{$currev};
						$ch = &toChar($currev); # need to convert digits to alphabets since netscape doesn't understand digit id
						print "<span style=\"background-color:$color\">";
						print "<a href=# \n";
						print "onclick=\"locking('$ch', event);return false;\"\n";
						print "onmouseover=\"return !showPopup('$ch', event);\"\n";
						print "onmouseout=\"hideCurrentPopup(); return false;\"\n";
						print ">";
						if($revMAPmatch{$currev}){
							print "<b>R</b>";
						}elsif($currev eq "grep"){
							print "<b>G</b>";
						}else{
							print "R";
						}
						print "</a></span>";
						$found = 1;
					}
					last if $toprev eq $currev;
				}
				if($found==0){
					if($flag==1){
						print " ";
					}else{
						print "<span style=\"background-color:#dddddd\"> </span>";
					}
				}
			}

			print "</td>";
			
			$color = Cvssearch::get_color($weight, 150);
			$line = &highlightquery($line);
			print "<td bgcolor=$color><pre><a href=\"$source$passparam#$i\" target=source>$line</a></td></tr>\n";
		}
		$i++;
	}
	print "</table>";
	#print an empty page so html can scroll to the last line
	print "<pre>";
	for($j=0;$j<60;$j++){
		print "\n";	
	}
	print "</pre>";
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
	$num = scalar(keys %lineMAPinfo);
	print Cvssearch::fileheader("<b>$num</b> Matched lines for <b>$name</b>", "Click on rev to stick/unstick");

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
	$word =~ s/(\d)/@{[chr($1+64)]}/ig;
	return $word;
}

#------------------
# display errors
#------------------
sub error{
	($mesg) = @_;
	print "<p><b class=red>$mesg</b>";
	exit(0);
}
