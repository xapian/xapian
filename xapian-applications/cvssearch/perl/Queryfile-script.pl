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

use CGI ':all';
use cvssearch;

#-------------------
# path variables
#-------------------
$CVSDATA = $ENV{"CVSDATA"}; # path where database content file is stored
$source = "./Source.cgi";
$match = "./Match.cgi";
$comments = "./Comments.cgi";

#-------------
# start html
#-------------

print "Content-type:text/html\n\n";
print "<html>\n";

if(param()){
	$dump = param("dump");
	$id = param("id");
	$displayname = param("displayname");
	$id = cvssearch::encode($id);
	$displayname = cvssearch::encode($displayname);

	$passparam = "?id=$id&dump=$dump&displayname=$displayname";
	
print <<_HTML_;
<frameset rows=\"60%,40%\">		
	<frameset cols=\"50%,50%\">
		<frame name=source src=$source$passparam>
		<frame name=match src=$match$passparam>
	</frameset>
	<frame name=comments src=$comments$passparam>
</frameset>

<noframes>
	<body bgcolor=\"#FFFFF0\">
	sorry this page requires frame to be displayed
	</body>
</noframes>
_HTML_
 	
}else{
	print "need to pass query and root as parameters"
}
print "</html>";
