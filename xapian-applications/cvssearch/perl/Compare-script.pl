use CGI qw(:standard);
use strict;
use Cvssearch;

my $pwd = `pwd`;
chomp $pwd;

my $cvsdata = Cvssearch::get_cvsdata();
my $cvscompare = "./Compare.cgi";
my $cvsquery = "$pwd/cvsquerydb";
my $cvsbuild = "$pwd/cvsbuilddb";
my $cvsmap = "$pwd/cvsmap";
my $ctrlA = chr(01);
my $ctrlB = chr(02);
my $ctrlC = chr(03);
my @class;
$class[0] = "class=\"e\"";
$class[1] = "class=\"o\"";

# ------------------------------------------------------------
# path where all our files are stored.
# ------------------------------------------------------------
if($cvsdata eq "") {
    print STDERR "WARNING: \$CVSDATA not set!\n";
    exit(1);
}

print header;

# ------------------------------------------------------------
# redirects to one of the subroutines
# depends on whether certain parameters are set or not
# ------------------------------------------------------------
if (0) {
} elsif (param("root") eq "") {
    compare_index();
} elsif (param("pkg") eq "") {
    compare_root_index(param("root"));
} elsif (param("fileid") eq "") {
    compare_pkg_index(param("root"), param("pkg"));
} elsif (param("version") eq "") {
    compare_file_index(param("root"), param("pkg"), param("fileid"));
} else {
    compare_file_version(param("root"), param("pkg"), param("fileid"), param("version"), param("latest"));
}

# ------------------------------------------------------------
# create a hash table of entries found in $cvsdata/CVSROOTS.
# ------------------------------------------------------------
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

# ------------------------------------------------------------
# no parameters are given.
# ------------------------------------------------------------
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

# ------------------------------------------------------------
# only the root parameter is given
# ------------------------------------------------------------
sub compare_root_index() {
    my ($root) = @_;

    print "<html>\n";
    print "<head>\n";
    print_title("root index $root");
    print_style_sheet();
    print "</head>\n";
    print "<body>\n";

    # ----------------------------------------
    # show a drop down menu
    # ----------------------------------------
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

    # ----------------------------------------
    # didn't obtain a valid cvsroot entry
    # ----------------------------------------
    if ($cvsroot eq "") {
        print start_html;
        print "the specified root directory does not correspond to a repository.\n";
        print "please check the file $cvsdata/CVSROOTS, it contains the mapping between repository path and root directory ";
        print "where cvssearch information for that repository are stored.";
        print end_html;
        exit(0);
    }

    # ----------------------------------------
    # found a repository, list all the packages
    # ----------------------------------------
    print "<h1 align=center>Repository $cvsroot</h1>\n";
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

# ------------------------------------------------------------
# list all the files that are indexed in a package
# ------------------------------------------------------------
sub compare_pkg_index {
    my ($root, $pkg) = @_;
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
    print_title ($pkg1);
    print_style_sheet();
    print "</head>\n";
    print "<body>\n";

    # ----------------------------------------
    # dump all the links here
    # ----------------------------------------
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
    
    # ----------------------------------------
    # append to the query string
    # ----------------------------------------
    my $command = "$cvsquery $root $pkg ";
    for ($i = 1; $i <= $#filenames + 1; $i++) {
        $command = $command . " -a $i";
    }

    # ----------------------------------------
    # output, only interested in the last
    # revision number and comment
    # ----------------------------------------
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
            # ----------------------------------------
            # only include the last version and comment
            # for each file
            # ----------------------------------------
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
    # ----------------------------------------
    # at this point 
    # @version == last version of all files (before indexing)
    # @comment == last comment of all files (before indexing)
    # ----------------------------------------

    close(RESULT);
    my $cvsroot = Cvssearch::read_cvsroot_dir($root, $cvsdata);
    print "<h1 align=center>$pkg1</h1>\n";
    print "<b>Up to ";
    print "<a href=\"$cvscompare?root=$root\">[$cvsroot]</a>/\n";
    print "</b><p>\n";

    print "Click on a file to display its revision history and see how lines from "; 
    print "early versions have been matched/aligned with lines in the latest version ";
    print "(so that commit comments are associated with the correct lines in the ";
    print "latest version).";

    print "<hr noshade>\n";
    print "<table  width=\"100%\" border=0 cellspacing=1 cellpadding=2>\n";
    print "<tr><td class=\"s\">File</td><td class=\"s\">Last Rev</td><td class=\"s\">Last CVS Comment</td></tr>\n";
    $i = 0;
    foreach (@filenames) {
        print "<tr>\n";
        print "<td ".$class[$i%2]."><a href=\"$cvscompare?root=$root&pkg=$pkg&fileid=". ($i+1)."\">$filenames[$i]</a></td>";
        print "<td ".$class[$i%2].">$versions[$i]</td>\n";
        print "<td ".$class[$i%2].">$comments[$i]</td>\n";
        print "</tr>\n";
        $i++;
    }
    print "</table>\n";
    print end_html;
}

# ------------------------------------------------------------
# display a list of revisions for a file
# ------------------------------------------------------------
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

    # ------------------------------------------------------------
    # get the filename
    # ------------------------------------------------------------
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
                # ----------------------------------------
                # throw away the package name part.
                # ----------------------------------------
                $filename = substr($filename, length($pkg1)+1, length($filename)-length($pkg1)-1);
            }
        }
    }

    if ($filename eq "") {
        # ----------------------------------------
        # woops.. didn't get a filename
        # ----------------------------------------
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
        } elsif (/$ctrlC/) {
            my @fields = split(/$ctrlC/);
            $version = $fields[0];
            $comment = $fields[1];
        } else {
            $comment = $comment."\n".$_;
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
        my $cvsroot = Cvssearch::read_cvsroot_dir($root, $cvsdata);
        print "<h1 align=\"center\">aligned diff outputs for <B>$filename</B></h1>\n";
        print "<b>Up to ";
        print "<a href=\"$cvscompare?root=$root\">[$cvsroot]</a>/\n";
        print "<a href=\"$cvscompare?pkg=$pkg&root=$root\">[$pkg1]</a>\n";
        print "</b><p>\n";
        
        print "<hr noshade>\n";
        print "Default branch: MAIN\n";
        print_compare_form($root, $pkg, $fileid, $versions[$#versions], $versions[0]);
        my $i;
        for ($i = 0; $i < $#versions; $i++) {
            print "<hr size=1 noshade>\n";
            print "<a href=\"$cvscompare?";
            print "fileid=$fileid&";
            print "pkg=$pkg&";
            print "root=$root&";
            print "version=$versions[$i]\"><b>$filename</b>: inserted/modified lines in commit ";
            print "$versions[$i+1] => $versions[$i] matched with corresponding lines in latest version</a><br>\n";
            print "<pre>$comments[$i]</pre>\n";
        }
        $i = $#versions;
        print "<hr size=1 noshade>\n";
        print "<a href=\"$cvscompare?";
        print "fileid=$fileid&";
        print "pkg=$pkg&";
        print "root=$root&";
        print "version=$versions[$i]\">initial version for <b>$filename</b></a><br>\n";
        print "<pre>$comments[$i]</pre>\n";

        print "<hr noshade>\n";
        print_compare_form($root, $pkg, $fileid, $versions[$#versions], $versions[0]);
    } else {
        print "There are no commits for the file <b>$filename</b>, no diff result is available.\n";
    }
    print end_html;
}

sub compare_file_version {
    my ($root, $pkg, $fileid, $version, $latest_version) = @_;
    $pkg=~tr/\//\_/;
    my $cvsroot = Cvssearch::read_cvsroot_dir($root, $cvsdata);
    if ($fileid eq "" || 
        $pkg eq "" ||
        $root eq "" ||
        $cvsdata eq "" ||
        $version eq "") {
        print start_html;
        print "The specified parameters are not valid\n"; 
        print end_html;
    }
    my $file ="";
    open (FILE, "$cvsquery $root $pkg -f $fileid -v $fileid|");
    while (<FILE>) {
        chomp;
        $file = $_;
        last;
    }

    if ($latest_version eq "") {
      while (<FILE>) {
        chomp;
        if (0) {
        } elsif (/$ctrlA/) {
        } else {
          $latest_version = $_;
        }
      }
    }
    close(FILE);

    if (Cvssearch::cmp_cvs_version($latest_version,$version) < 0) {
       print start_html;
       print "the specified latest version $latest_version is older than the version $version\n";
       print end_html;
       return;
    }

    print "<html>\n";
    print "<head>\n";
    print_title("aligned diff output for $file:version $version");
    print_style_sheet();
    print "</head>\n";
    print "<body>\n";
    chdir ("$cvsdata/$root/src");
    open (OUTPUT, "$cvsmap -d $cvsroot -db $cvsdata/$root/db/$pkg.db/$pkg.db -html $fileid $version -r $latest_version $file |");
    while (<OUTPUT>) {
      print $_;
    }
    close(OUTPUT);
    chdir ($pwd);
    print_compare_form($root, $pkg, $fileid, $version, $latest_version);
    print "</body>\n";
    print "</html>\n";
}

sub usage {
    print << "EOF";
Compare.cgi 1.0 (2001-3-15)
Usage URL: http://www.example.com/cgi-bin/Compare.cgi
EOF
exit 0;
}

sub print_compare_form {
    my ($root, $pkg, $fileid, $version, $latest_version) = @_;
    print "<form action=./Compare.cgi>\n";
    print "<input type=hidden name=root value=$root>\n";
    print "<input type=hidden name=pkg value=$pkg>\n";
    print "<input type=hidden name=fileid value=$fileid>\n";
    print "This form allows you to see the differences occurred during commit ";
    print "<input type=text size=5 name=version value=\"$version\">,<br>\n";
    print "and the propagation of the affected lines to version ";
    print "<input type=text size=5 name=latest  value=\"$latest_version\">\n";
    print "<input type=submit value=\"Compare\"><br>\n";
    print "<font size=-1>(the latest version prior to build database will be used if this field is empty).</font><br>\n";
    print "</form>\n";
}

sub  print_title {
    my ($title) = @_;
    print "<title>$title</title>\n";
}

sub print_style_sheet {
    print "<style type=\"text/css\">\n";
    print "body  {background-color:#EEEEEE;}\n";
    print "table {background-color:#FFFFFF;}\n";
#    print "td    {white-space:pre; overflow:hidden;font-family:'sans serif',courier;}\n";
    print "td    {white-space:pre; overflow:hidden;}\n";
    print ".e {background-color:#ffffff;}\n";
    print ".a {background-color:#CCCCFF;}\n";
    print ".o {background-color:#ccccee;}\n";
    print ".c {background-color:#99FF99;}\n";
    print ".s {background-color:#3366CC; color:#FFFFFF;}\n";
    print ".d {background-color:#FF9999;}\n";
    print ".n {background-color:#EEEEEE;}\n";
    print "</style>\n";
}
