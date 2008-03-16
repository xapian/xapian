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
my $match = "./MatchComment.cgi";
my $top = "./TopComment.cgi";
my $querycomment = "./QueryComment.cgi";
my $source = "./SourceComment.cgi";
my $related = "./CommitSearch.cgi";
my $cvscompare ="./Compare.cgi";
my $cvsquery = "./cvsquerydb";
my $ctrlA = chr(01);
my $ctrlB = chr(02);
my $ctrlC = chr(03);

my $cvsdata = Cvssearch::get_cvsdata();
my @class;
$class[0] = "class=\"e\"";
$class[1] = "class=\"o\"";

my $root = Cvssearch::sanitise_root(param("root"));

# ------------------------------------------------------------
# redirects to one of the subroutines
# depends on whether certain parameters are set or not
# ------------------------------------------------------------
if (0) {
} elsif ($root eq "") {
    system ("$cvscompare");
} elsif (param("pkg") eq "") {
    system ("$cvscompare");
} elsif (param("id") eq "") {
    print header;
    commit_pkg_index($root, param("pkg"));
} else {
    print header;
    commit($root, param("pkg"), param("id"), param("symbol"));
}

sub commit {
	my ($root, $pkg, $id, $symbol) = @_;
    my @file_ids;
    my @file_names;
    my @revisions;

    open (QUERY, "$cvsquery $root $pkg -A $id |");
    while (<QUERY>) {
        chomp;
        if (0) {
        } elsif (/$ctrlC/) {
            my @fields = split(/$ctrlC/);
            @file_ids   = (@file_ids,   $fields[0]);
            @file_names = (@file_names, $fields[1]);
            @revisions  = (@revisions,  $fields[2]);
            last;
        }
    }
    close (QUERY);

	my $fileid = shift @file_ids;
    my $revision = shift @revisions;
	my $passparam = "?id=$id&root=$root&pkg=$pkg&symbol=$symbol";

    print <<_HTML_;	
<html>
<head>
</head>
<frameset rows=\"90, 45%, *\">
	<frame name=\"t\" src=$top$passparam>
    <frame name=\"m\" src=$match$passparam>
    <frame name=\"s\" src=$related?query=$pkg\@$id%20in:$pkg&root=$root>
</frameset>
<noframes>
	<body bgcolor=\"#FFFFF0\">
	sorry this page requires frame to be displayed
	</body>
</noframes>
</html>
_HTML_
}

# ------------------------------------------------------------
# list all the files that are indexed in a package
# ------------------------------------------------------------
sub commit_pkg_index {
    my ($root, $pkg) = @_;
    my $i = 0;
    print "<html>\n";
    print "<head>\n";
    print "<title>$pkg</title>\n";
    Cvssearch::print_style_sheet();
    print_javascript ($root, $pkg);
    print "</head>\n";
    print "<body>\n";
    
    # ----------------------------------------
    # append to the query string
    # ----------------------------------------
    my $command = "$cvsquery $root $pkg -All";
    my $comment = "";
    my @fileids;
    my @filenames;
    my @revisions;
    my $commitid = 1;
    my $cvsroot = Cvssearch::read_cvsroot_dir($root, $cvsdata);
    print "<h1 align=center>Commits for $pkg</h1>\n";
    print "<b>Up to ";
    print "<a href=\"$cvscompare?root=$root\">[$cvsroot]</a>\n";
    print "</b><p>\n";
    
#    print "Click on a file to display its revision history and see how lines from "; 
#    print "early versions have been matched/aligned with lines in the latest version ";
#    print "(so that commit comments are associated with the correct lines in the ";
#    print "latest version).<br>\n";

#    print "Click on a revision to display how lines from "; 
#    print "this revision of this file has been matched/aligned with lines in the latest version ";
#    print "(so that commit comments are associated with the correct lines in the ";
#    print "latest version).<br>\n";

    print "Click on <b>Browse Code</b> to display all the lines involved in that commit "; 
    print "and have been matched/aligned with lines in the latest version ";
    print "(so that the commit comments are associated with the correct lines in the ";
    print "latest version).<br>\n";
    
    print "<hr noshade>\n";

    # ----------------------------------------
    # output, only interested in the last
    # revision number and comment
    # ----------------------------------------
    open (RESULT, "$command |");
    print "<table  width=\"100%\" border=0 cellspacing=0 cellpadding=2>\n";
    print "<tr><td class=s colspan=3>Commit Comments</td></tr>\n";
    while (<RESULT>) {
        chomp;
        if (0) {
        } elsif (/$ctrlC/) {
            my ($fileid, $filename, $revision) = split(/$ctrlC/);
            @fileids   = (@fileids,   $fileid);
            @filenames = (@filenames, $filename);
            @revisions = (@revisions, $revision);
        } elsif (/$ctrlB/) {
            my $count = $#filenames + 2;
            my $printed_comment = 0;
            print "<tr valign=top>\n";
            print "<td $class[$i%2] colspan=2>";
            print "<a href=\"$querycomment?root=$root&pkg=$pkg&id=$commitid\" target=_top><pre>Browse Code</pre></a></td>\n";
            print "<td $class[$i%2] rowspan=$count><pre>$comment</pre></td>\n";
            print "</tr>\n";
            foreach (@fileids) {
                my $filename = shift @filenames;
                my $revision = shift @revisions;
                if ($pkg == substr($filename, 0, length($pkg))) {
                    # ----------------------------------------
                    # throw away the package name part.
                    # ----------------------------------------
                    $filename = substr($filename, length($pkg)+1, length($filename)-length($pkg)-1);
                }
                print "<tr>\n";
                print "<td $class[$i%2]>$filename</td>\n";
                print "<td $class[$i%2]>$revision</td>\n";
#                print "<td $class[$i%2]><a href=# onclick=\"return f($_);\">$filename</a></td>\n";
#                print "<td $class[$i%2]><a href=# onclick=\"return r($_, \'$revision\');\">$revision</a></td>\n";
                print "</tr>\n";
            }
            $commitid++;
            $i++;
            $comment = "";
            @filenames = ();
            @fileids = ();
            @revisions = ();
        } else {
            $comment = $comment."\n".$_;
        }
    }
    print "</table>\n";
}

sub usage {
    print "Commit.cgi 1.0 (2001-3-15)\n";
    print "Usage URL: http://www.example.com/cgi-bin/Commit.cgi\n";
    exit 0;
}

sub print_javascript {
    my ($root, $pkg) = @_;
    # ----------------------------------------
    # print javascript for calling popups in
    # shorthand notation
    # ----------------------------------------
    
    print <<_SCRIPT_;
<script language="JavaScript">

function c(id) {
    var link = "$querycomment?root=$root&pkg=$pkg&id=" + id;
    if (this.location.href != link) {
        this.location.href = link;
    }
    return false;
}

function r(fileid, rev){
    var link = "$cvscompare?root=$root&pkg=$pkg&fileid=" + fileid +"&short=1&version="+ rev;
    if (this.location.href != link) {
        this.location.href = link;
    }
    return false;
}

function f(fileid){
    var link = "$cvscompare?root=$root&pkg=$pkg&fileid=" + fileid +"&short=1";
    if (this.location.href != link) {
        this.location.href = link;
    }
    return false;
}

</script>
_SCRIPT_
}
