#-------------------------------------------------------------------
# some stuff about cvssearch
# This produces Header for matched files
#
# Author: Annie - anniec@cse.unsw.edu.au
# Date: Feb 16 2001
#-------------------------------------------------------------------

use CGI ':all';
use Cvssearch;

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
b {font-family:arial,sans-serif; color:#0066cc}
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
	$did = Cvssearch::encode($id);
	$displayname = param("displayname");
	$displayname = Cvssearch::decode($displayname);

	$id = Cvssearch::decode($id);
	my ($root, $db, $fileid) = split / /, $id;
print <<_HTML_;
<a href="./Compare.cgi?root=$root&pkg=$db&fileid=$fileid" target=s><b>$displayname</b></a>
_HTML_
}
print "</body></html>";
