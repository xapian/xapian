use strict;
use Cvssearch;

my $cvsquery = "./cvsquery";
my $cvsdata = Cvssearch::get_cvsdata();
my $cvsroot_dir;

sub usage();

# ------------------------------------------------------------
# path where all our files are stored.
# if $cvsdata is not set, use current directory instead.
# ------------------------------------------------------------
if($cvsdata eq "") {
    print STDERR "WARNING: \$CVSDATA not set and ";
    print STDERR "cvssearch.conf has not been created!\n";
    exit(1);
}

if ($#ARGV < 0) {
    usage();
}

if ($ARGV[0] eq "-h") {
    usage();
}

$_ = $ARGV[1];

my $root_dir = $ARGV[0];
my $package = Cvssearch::strip_last_slash($ARGV[1]);
$package =~ tr!/!_!;

$ARGV[0]= "$cvsdata/$root_dir/db/$package.db/$package.db";
# ------------------------------------------------------------
# call cvsmap-script with the same parameters
# ------------------------------------------------------------
system ("$cvsquery @ARGV");


sub usage()
{

print << "EOF";
cvsquerydb 1.0 (2001-2-26)
Usage: $0 root_dir package [Options] [Options] ...

Options:
  -h                     print out this message
  -f file_id             query for filename given a file id
  -F filename            query for file_id given a file name
  -All                   query for all filename, revision pairs for each commit
  -A commit_id           query for all filename, revision pairs in a commit
  -a file_id             query for all revision, comment pairs in a file
  -C commit_id           query for cvs comments associated in a commit
  -c file_id revision    query for cvs comments from a file_id and a revision
  -r file_id line        query for revisions from a file_id and a line
  -l file_id revision    query for lines from a file_id and a revision
  -v file_id             query for all revisions
EOF
exit 0;
}
