# -* perl *-

use CGI qw(:standard);
use strict;
use Cvssearch;

my $cvsdata = &cvssearch::get_cvsdata();
my $cvscompare = "./cvscompare.cgi";
my $cvsquery = "./cvsquerydb";
my $cvsbuild = "./cvsbuilddb";
my $cvsmap = "./cvsmap";
my $ctrlA = chr(01);
my $ctrlB = chr(02);
my $ctrlC = chr(03);
my @class;
$class[0] = "class=\"e\"";
$class[1] = "class=\"o\"";

sub compare_index();

# ------------------------------------------------------------
# path where all our files are stored.
# ------------------------------------------------------------
if($cvsdata) {
}else{
    print STDERR "WARNING: \$CVSDATA not set!\n";
    exit(1);
}

if ($#ARGV < 0) {
    usage();
}

print header;

if (param("root") eq "") {
    compare_index();
} elsif (param("pkg") eq "") {
    compare_root_index(param("root"));
} elsif (param("fileid") eq "") {
    compare_pkg_index(param("root"), param("pkg"));
} elsif (param("version") eq "") {
    compare_file_index(param("root"), param("pkg"), param("fileid"));
} else {
    compare_file_version(param("root"), param("pkg"), param("fileid"), param("version"));
}

sub read_cvs_roots() {
    open (CVSROOTS, "<$cvsdata/CVSROOTS");
    my %roots;
    while (<CVSROOTS>) {
        chomp;
        my @fields = split(/ /);
        $roots{$fields[0]} = $fields[1];
    }
    close (CVSROOTS);
    return %roots;
}

sub compare_index() {
    my @roots = read_cvs_roots();
    if ($#roots >= 1) {
        compare_root_index($roots[1]);
    } else {
        print start_html;
        print "no packages have been built, please run $cvsbuild -d \$CVSROOT packages... to build packages";
        print end_html;
        exit(0);
    }
}

sub compare_root_index {
    my ($root) = @_;

    print "<html>\n";
    print "<head>\n";
    print_title("root index $root");
    print_style_sheet();
    print "</head>\n";
    print "<body>\n";

    print "<form action=$cvscompare>\n";
    print "<b>Select Repository: </b>\n";
    print "<select name=root>\n";
    my %roots = read_cvs_roots();
    my $cvsroot;
    foreach (keys %roots) {
        print "<option value=$roots{$_}>$_</option>\n";
        if ($root eq $roots{$_}) {
            $cvsroot = $_;
        }
    }
    print "</select><input type=submit></form>\n";

    if ($cvsroot eq "") {
        print start_html;
        print "the specified root directory does not correspond to a repository.\n";
        print "please check the file $cvsdata/CVSROOTS, it contains the mapping between repository path and root directory ";
        print "where cvssearch information for that repository are stored.";
        print end_html;
        exit(0);
    }

    print "<H1 align=center>Repository $cvsroot</H1>\n";
    open (DBCONTENT, "<$cvsdata/$root/dbcontent");
    print "<table  width=\"100%\" border=0 cellspacing=1 cellpadding=2>\n";
    print "<tr><td class=\"s\">Package</td></tr>\n";
    my $i = 0;
    while (<DBCONTENT>) {
        chomp;
        my $pkg1 = $_;
        $pkg1 =~tr/\_/\//;
        print "<tr>\n";
        print "<td $class[$i%2]><a href=\"$cvscompare?root=$root&pkg=$pkg1\">$pkg1</a></td>";
        print "</tr>\n";
        $i++;
    }
    close (DBCONTENT);
    print "</table>\n";
    print end_html;
}

sub compare_pkg_index {
    my ($root, $pkg) = @_;
    # ----------------------------------------
    # dump all the links here
    # ----------------------------------------
    $pkg  =~tr/\//\_/;
    my $pkg1 = $pkg;
    my $i;
    my $filename;
    my @filenames;
    my $version;
    my $comment;
    my @versions;
    my @comments;
    $pkg1 =~tr/\_/\//;

    print "<html>\n";
    print "<head>\n";
    print "<TITLE>package $pkg1</TITLE>\n";
    print_style_sheet();
    print "</head>\n";
    print "<body>\n";

    open (OFFSET, "<$cvsdata/$root/db/$pkg.offset");
    $i = 1;
    while (<OFFSET>) {
        chomp;
        my @fields = split(/ /);
        $filename = $fields[0];
        if ($pkg1 == substr($filename, 0, length($pkg1))) {
            $filename = substr($filename, length($pkg1)+1, length($filename)-length($pkg1)-1);
        }
        @filenames = (@filenames, $filename);
        $i++;
    }
    close (OFFSET);
    
    my $command = "$cvsquery $root $pkg ";
    for ($i = 1; $i <= $#filenames + 1; $i++) {
        $command = $command . " -a $i";
    }

    open (RESULT, "$command |");
    while (<RESULT>) {
        chomp;
        if (0) {
        } elsif (/$ctrlB/) {
            my $line = $_;
            my @fields = split(/$ctrlB/);
            $_ = $line;
            if (/$ctrlC/) {
                my @fieldss = split(/$ctrlC/, $fields[0]);
                $version = $fieldss[0];

                if (0) {
                } elsif ($#fieldss == 1) {
                    $comment= $fieldss[1];
                } elsif ($#fieldss == 0) {
                    $comment="*** empty log message ***";
                }
            }
        } elsif (/$ctrlA/) {
            @versions = (@versions, $version);
            @comments = (@comments, $comment);
        } else {
            my @fields = split(/$ctrlC/);
            if (0) {
            } elsif ($#fields == 1) {
                $version = $fields[0];
                $comment = $fields[1];
            } elsif ($#fields == 0) {
                $comment = $comment."\n".$fields[0];
            }
        }
    }
    close(RESULT);
    my $cvsroot = &cvssearch::read_cvsroot_dir($root, $cvsdata);
    print "<H1 align=center>$pkg1</H1>\n";
    print "<b>Up to ";
    print "<a href=\"$cvscompare?root=$root\">[$cvsroot]</a>/\n";
    print "</b><p>\n";
    print "Click on a file to display its revision history and to get a chance to display aligned diffs between consecutive revisions.\n";
    print "<HR NOSHADE>\n";
    print "<table  width=\"100%\" border=0 cellspacing=1 cellpadding=2>\n";
    print "<tr><td class=\"s\">File</td><td class=\"s\">Last Rev</td><td class=\"s\">Last CVS Comment</td></tr>\n";
    $i = 0;
    foreach (@filenames) {
        my @class;
        $class[0] = "class=\"e\"";
        $class[1] = "class=\"o\"";
        print "<tr>\n";
        print "<td $class[$i%2]><a href=\"$cvscompare?root=$root&pkg=$pkg&fileid=". ($i+1)."\">$filenames[$i]</a></td>";
        print "<td $class[$i%2]>$versions[$i]</td>\n";
        print "<td $class[$i%2]>$comments[$i]</td>\n";
        print "</tr>\n";
        $i++;
    }
    print "</table>\n";
    print end_html;
}

sub compare_file_index {
    my ($root, $pkg, $fileid) = @_;
    # ----------------------------------------
    # dump all the links here
    # ----------------------------------------
    $pkg  =~tr/\//\_/;    
    open (FILE, "$cvsquery $root $pkg -f $fileid -a $fileid |");
    my $version;
    my $comment;
    my @versions;
    my @comments;
    my $filename;
    my $pkg1;

    while (<FILE>) {
        chomp;
        if (0) {
        } elsif (/$ctrlA/) {
            last;
        } else {
            $filename = $_;
            $pkg1 = $pkg;
            $pkg1 =~tr/\_/\//;
            if ($pkg1 == substr($filename, 0, length($pkg1))) {
                $filename = substr($filename, length($pkg1)+1, length($filename)-length($pkg1)-1);
            }
        }
    }
    if ($filename eq "") {
        print start_html;
        print "The specified parameters are not valid\n"; 
        print end_html;
        return;
    }

    while (<FILE>) {
        chomp;
        if (0) {
        } elsif (/$ctrlB/) {
            my $line = $_;
            my @fields = split(/$ctrlB/);
            $_ = $line;
            if (/$ctrlC/) {
                my @fieldss = split(/$ctrlC/, $fields[0]);
                $version = $fieldss[0];

                if (0) {
                } elsif ($#fieldss == 1) {
                    $comment= $fieldss[1];
                } elsif ($#fieldss == 0) {
                    $comment="*** empty log message ***";
                }
            }
            @versions = ($version, @versions);
            @comments = ($comment, @comments);
        } elsif (/$ctrlA/) {
            last;
        } else {
            my @fields = split(/$ctrlC/);
            if (0) {
            } elsif ($#fields == 1) {
                $version = $fields[0];
                $comment = $fields[1];
            } elsif ($#fields == 0) {
                $comment = $comment."\n".$fields[0];
            }
        }
    }
    close(FILE);
    
    print "<html>\n";
    print "<head>\n";
    print_title("aligned diff outputs for $filename");
    print_style_sheet();
    print "</head>\n";
    print "<body>\n";
    
    if ($#versions >= 0) {
        my $cvsroot = &cvssearch::read_cvsroot_dir($root, $cvsdata);
        print "<H1 align=\"center\">aligned diff outputs for <B>$filename</B></H1>\n";
        print "<b>Up to ";
        print "<a href=\"$cvscompare?root=$root\">[$cvsroot]</a>/\n";
        print "<a href=\"$cvscompare?pkg=$pkg&root=$root\">[$pkg1]</a>\n";
        print "</b><p>\n";
        
        print "<HR NOSHADE>\n";
        print "Default branch: MAIN\n";
        my $i;
        for ($i = 0; $i < $#versions; $i++) {
            print "<HR size=1 NOSHADE>\n";
            print "<a href=\"$cvscompare?";
            print "fileid=$fileid&";
            print "pkg=$pkg&";
            print "root=$root&";
            print "version=$versions[$i]\">aligned diff for <b>$filename</b> between ";
            print "version $versions[$i+1] & $versions[$i]</a><br>\n";
            print "<pre>$comments[$i]</pre>\n";
        }
        $i = $#versions;
        print "<HR size=1 NOSHADE>\n";
        print "<a href=\"$cvscompare?";
        print "fileid=$fileid&";
        print "pkg=$pkg&";
        print "root=$root&";
        print "version=$versions[$i]\">initial version for <b>$filename</b></a><br>\n";
        print "<pre>$comments[$i]</pre>\n";

        print "<HR NOSHADE>\n";
    } else {
        print "There are less than two commits for the file <b>$filename</b>, no diff result is available.\n";
    }
    print end_html;
}

sub dummy_page {
    
}

sub compare_file_version {
    my ($root, $pkg, $fileid, $version) = @_;
    $pkg=~tr/\//\_/;
    my $cvsroot = &cvssearch::read_cvsroot_dir($root, $cvsdata);
    if ($fileid eq "" || 
        $pkg eq "" ||
        $root eq "" ||
        $cvsdata eq "") {
        
        dummy_page();
    }
    print STDERR "$fileid $pkg, $root, $version $cvsroot\n";

    open (FILE, "$cvsquery $root $pkg -f $fileid |");
    while (<FILE>) {
        chomp;
        my $file = $_;
        open (OUTPUT, "$cvsmap -d $cvsroot -db $cvsdata/$root/db/$pkg.db/$pkg.db -html $fileid $version $cvsdata/$root/src/$file |");
        while (<OUTPUT>) {
            print $_;
        }
        close(OUTPUT);
        last;
    }
    close(FILE);
}

sub usage {

}

sub  print_title {
    my $title = @_;
    print "<TITLE>$title</TITLE>\n";
}

sub print_style_sheet {
    print "<STYLE TYPE-\"type/css\">\n";
    print "body  {background-color:#EEEEEE;}\n";
    print "table {background-color:#FFFFFF;}\n";
    print "td    {white-space:pre; overflow:hidden;}\n";
    print ".e {background-color:#ffffff;}\n";
    print ".o {background-color:#ccccee;}\n";
    print ".s {background-color:#3366CC; color:#FFFFFF;}\n";
    print "</STYLE>\n";
}
