# -* perl *-

use strict;

my $CVSDATA;
# ------------------------------------------------------------
# path where all our files are stored.
# if $CVSDATA is not set, use current directory instead.
# ------------------------------------------------------------
$CVSDATA = $ENV{"CVSDATA"}; 

if($CVSDATA) {
}else{
    print STDERR "WARNING: \$CVSDATA not set! use current directory.\n";
    $CVSDATA = ".";
}

if ($#ARGV < 0) {
    usage();
}

$_ = $ARGV[0];
$ARGV[0]=~tr/\//\_/;
$ARGV[0] = "$CVSDATA/database/$ARGV[0]/$ARGV[0].db";

# ------------------------------------------------------------
# call cvsmap-script with the same parameters
# ------------------------------------------------------------
system ("cvsquery @ARGV");

sub usage()
{
print << "EOF";
cvsquery-script 0.1 (2001-2-26)
Usage: cvsquery-script package [Options] [Options] ...

Options:
  -h                     print out this message
  -c file_id revision    query for cvs comments from a file_id and a revision
  -r file_id line        query for revisions from a file_id and a line
  -l file_id revision    query for lines from a file_id and a revision
  -a file_id             query for all revision,comment pairs
EOF
exit 0;
}

