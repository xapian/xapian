use strict;
use Cvssearch;

my $cvsquery = "./cvsquery";
my $cvsdata = Cvssearch::get_cvsdata();
my $cvsroot_dir;

# ------------------------------------------------------------
# path where all our files are stored.
# if $cvsdata is not set, use current directory instead.
# ------------------------------------------------------------
if($cvsdata eq "") {
    print STDERR "WARNING: \$CVSDATA not set and \n";
    print STDERR "cvssearch.conf is not created. !\n";
    exit(1);
}

if ($#ARGV < 0) {
    usage();
}

$_ = $ARGV[1];
$ARGV[1]=~tr/\//\_/;
$ARGV[1] = "$cvsdata/$ARGV[0]/db/$ARGV[1].db/$ARGV[1].db";

shift @ARGV;
# ------------------------------------------------------------
# call cvsmap-script with the same parameters
# ------------------------------------------------------------
system ("$cvsquery @ARGV");


sub usage()
{
print << "EOF";
cvsquery-script 0.1 (2001-2-26)
Usage: cvsquerydb root package [Options] [Options] ...

Options:
  -h                     print out this message
  -c file_id revision    query for cvs comments from a file_id and a revision
  -r file_id line        query for revisions from a file_id and a line
  -l file_id revision    query for lines from a file_id and a revision
  -a file_id             query for all revision,comment pairs
EOF
exit 0;
}
