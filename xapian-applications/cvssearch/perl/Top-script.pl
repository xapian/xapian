#-------------------------------------------------------------------
# some stuff about cvssearch
# This produces Header for matched files with the filename
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
	$displayname = param("displayname");
	$displayname = Cvssearch::decode($displayname);

print <<_HTML_;
<b>$displayname</b>
_HTML_
}
print "</body></html>";
