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
my $cvsdata = Cvssearch::get_cvsdata();
my $ctrlA = chr(01);
my $ctrlB = chr(02);
my $ctrlC = chr(03);
my $related = "./CommitSearch.cgi";
my $cvscompare = "./Compare.cgi";
my $cvsquery = "./cvsquerydb";
my $querycomment = "./QueryComment.cgi";

#-------------
# start html
#-------------

print header;
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
if (param()) {
    my $commit_id = param("id");
    my $root = Cvssearch::sanitise_root(param("root"));
    my $pkg = param("pkg");
    my $cvsroot = Cvssearch::read_cvsroot_dir($root, $cvsdata);

    print "<body>\n";
    print "<table  width=\"100%\" border=0 cellspacing=1 cellpadding=2>\n";
    print "<tr>\n";

    print "<td align=left>";
    print "<b>Up to <a href=\"$cvscompare?root=$root\" target=_top>[$cvsroot]</a> ";
    print "<a href=\"$cvscompare?root=$root&pkg=$pkg\" target=\"s\">[$pkg]</a></b>\n";
    print "<td align=center><b>Similar Commits in [<a href=\"$related?query=$pkg\@$commit_id%20in:$pkg&root=$root\" target=s>$pkg</a> / <a href=\"$related?query=$pkg\@$commit_id&root=$root\" target=s>All Packages</a>]</b></td>\n";
    print "<td align=right><a href=\"./Query.cgi\" target=_top>Search Again</a></td>\n";
#    print "<tr></tr>\n";
#    print "<td align=left><b>Similar Commits in [<a href=\"$related?query=$pkg\@$commit_id%20in:$pkg&root=$root\" target=s>$pkg</a> / <a href=\"$related?query=$pkg\@$commit_id&root=$root\" target=s>All Packages</a>]</b></td>\n";
    print "</tr>\n";
    print "</table>\n";

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
