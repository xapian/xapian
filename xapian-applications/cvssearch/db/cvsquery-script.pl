# -* perl *-

use strict;

my $cvsdata = strip_last_slash($ENV{"CVSDATA"});
my $cvsroot_dir = "";
my $i;
my @modules;

# ------------------------------------------------------------
# path where all our files are stored.
# if $cvsdata is not set, use current directory instead.
# ------------------------------------------------------------
if($cvsdata) {
}else{
    print STDERR "WARNING: \$CVSDATA not set!\n";
    exit(1);
}

if ($#ARGV < 0) {
    usage();
}

$_ = $ARGV[1];
$ARGV[1]=~tr/\//\_/;
$ARGV[0] = "$cvsdata/$ARGV[0]";
$ARGV[1] = "$ARGV[0]/db/$ARGV[1]/$ARGV[1].db";

# ------------------------------------------------------------
# call cvsmap-script with the same parameters
# ------------------------------------------------------------
system ("cvsquery @ARGV");


sub usage()
{
print << "EOF";
cvsquery-script 0.1 (2001-2-26)
Usage: cvsquery-script root package [Options] [Options] ...

Options:
  -h                     print out this message
  -c file_id revision    query for cvs comments from a file_id and a revision
  -r file_id line        query for revisions from a file_id and a line
  -l file_id revision    query for lines from a file_id and a revision
  -a file_id             query for all revision,comment pairs
EOF
exit 0;
}

sub strip_last_slash() {
    my $path = $_[0];
    my $path_length = length($path);
    my $slash = rindex($path, "/");
    if ($slash == $path_length-1) {
        return substr($path, 0, $path_length-1);
    } else {
        return $path;
    }
   
}
