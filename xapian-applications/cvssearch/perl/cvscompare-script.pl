# -* perl *-

use CGI qw(:standard);
use strict;
use cvssearch;

my $cvsdata = &cvssearch::get_cvsdata();
my $cvsquery = "./cvsquery-script";
my $cvsmap = "./cvsmap";
my $ctrlA = chr(01);
my $ctrlB = chr(02);
my $ctrlC = chr(03);

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

if (param("version") ne "") {
    cvs_compare_files(param("fileid"), param("pkg"), param("root"), param("version"));
} else {
    cvs_compare_index(param("fileid"), param("pkg"), param("root"));
}

sub cvs_compare_index {
    my ($fileid, $pkg, $root) = @_;
    # ----------------------------------------
    # dump all the links here
    # ----------------------------------------
    print start_html("Index");
    open (FILE, "$cvsquery $root $pkg -a $fileid |");
    my $version;
    my $comments;

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
                    $comments= $fieldss[1];
                }
            }
            print "<P><A HREF=\"./cvscompare.cgi?";
            print "fileid=$fileid&";
            print "pkg=$pkg&";
            print "root=$root&";
            print "version=$version\">$version</A></P>\n";
            print "$comments\n";
        } elsif (/$ctrlA/) {
            last;
        } else {
            my @fields = split(/$ctrlC/);
            if (0) {
            } elsif ($#fields == 1) {
                $version = $fields[0];
                $comments = $fields[1];
            } elsif ($#fields == 0) {
                $comments = $comments."\n".$fields[1];
            }
        }
    }
    print end_html;
    close(FILE);
}

sub dummy_page {
    
}

sub cvs_compare_files {
    my ($fileid, $pkg, $root, $version) = @_;
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
        system ("$cvsmap -d $cvsroot -db $cvsdata/$root/db/$pkg.db/$pkg.db -html $fileid $version $cvsdata/$root/src/$file");
        last;
    }
    close(FILE);

}

sub usage {

}
