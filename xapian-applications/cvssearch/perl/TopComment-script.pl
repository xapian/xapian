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
my $ctrlA = chr(01);
my $ctrlB = chr(02);
my $ctrlC = chr(03);

my $cvsquery = "./cvsquerydb";

#-------------
# start html
#-------------

print "Content-type:text/html\n\n";
print "<html>\n";
print "<head>\n";

#---------------
# style sheet
#---------------
Cvssearch::print_style_sheet();
print "</head>\n";
#----------------------------------------
# Parse Parameters
#----------------------------------------
if(param()){
    my $commit_id = param("id");
    my $root = param("root");
    my $pkg = param("pkg");
    print "<body>\n";
    print "<table  width=\"100%\" border=0 cellspacing=1 cellpadding=2>\n";
    print "<tr>\n";
    print "<td align=left><a href=\"./Compare.cgi?root=$root&pkg=$pkg\" target=\"s\"><b>$pkg</b></a></td>\n";
    print "<td align=right><a href=\"./Query.cgi\" target=_top>Search Again</a></td>\n";
    print "</tr></table>\n";

    # ----------------------------------------
    # print cvs comment
    # ----------------------------------------
    open (QUERY, "$cvsquery $root $pkg -C $commit_id|");
    my $comment = "";
    while (<QUERY>) {
        chomp;
        if (0) {
        } elsif (/$ctrlA/) {
            last;
        } else {
            $comment .= "$_\n";
        }
    }
    close (QUERY);
    print "<pre class=popuplink>CVS comment:\n$comment</pre>\n";
}
print "</body></html>";
