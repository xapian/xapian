#-------------------------------------------------------------------
# some stuff about cvssearch
# This produces Header for matched files
#
# Author: Annie - anniec@cse.unsw.edu.au
# Date: Feb 16 2001
#-------------------------------------------------------------------

use strict;
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
Cvssearch::print_style_sheet();
print "</header>\n";
#----------------------------------------
# Parse Parameters
#----------------------------------------
if(param()){
  my $dump = param("dump");
  my $id = param("id");
  my $did = Cvssearch::encode($id);
  my $displayname = param("displayname");
  $displayname = Cvssearch::decode($displayname);
  my $query = "";
  my $cvsdata = Cvssearch::get_cvsdata();
  my %dirMAProot = Cvssearch::read_cvsroots($cvsdata);
  $id = Cvssearch::decode($id);
  my ($root, $db, $fileid) = split / /, $id;
  $db =~tr/\_/\//;
  print "<body>\n";
  print "<table  width=\"100%\" border=0 cellspacing=1 cellpadding=2>\n";
  print "<tr>\n";
  print "<td align=left><a href=\"./Compare.cgi?root=$root&pkg=$db&fileid=$fileid\" target=\"s\"><b>$displayname</b></a></td>\n";
  print "<td align=right><a href=\"./Query.cgi\" target=_top>Search Again</a></td>\n";
  print "</tr></table>\n";
}
print "</body></html>";
