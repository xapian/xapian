#-------------------------------------------------------------------
# some stuff about cvssearch
# This produces a html with frames linking to other frames:
# 1. match.cgi
# 2. source.cgi
# 3. comments.cgi
#
# Things needed to pass to this script
# 1. dump file
# 2. file id (rootx project id)
# 
# Structure:
# 1. create a frame structure
# 2. pass dump and id to all other frames
#
# Author: Annie - anniec@cse.unsw.edu.au
# Date: Feb 25 2001
#-------------------------------------------------------------------

use strict;
use CGI ':all';
use Cvssearch;

#-------------------
# path variables
#-------------------
my $source = "./Compare.cgi";
my $match = "./MatchComment.cgi";
my $top = "./TopComment.cgi";

#-------------
# start html
#-------------

print "Content-type:text/html\n\n";
print "<html>\n";

if(param()){
	my $id = param("id");
    my $root = param ("root");
    my $pkg = param ("pkg");
    my $symbol = param ("symbol");
	my $passparam = "?id=$id&root=$root&pkg=$pkg&symbol=$symbol";

print <<_HTML_;	
<head>
</head>
<frameset rows=\"90, 45%, *\">
	<frame name=\"t\" src=$top$passparam>
	<frame name=\"m\" src=$match$passparam>
    <frame name=\"s\" src=$source$passparam>
</frameset>
<noframes>
	<body bgcolor=\"#FFFFF0\">
	sorry this page requires frame to be displayed
	</body>
</noframes>
_HTML_
 	
}else{
	print "need to pass cvs commit id, root, pkg as parameters"
}
print "</html>";
