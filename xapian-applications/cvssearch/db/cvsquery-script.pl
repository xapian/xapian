# -* perl *-

use strict;

my $cvsdata = strip_last_slash($ENV{"CVSDATA"});
my $cvsroot = strip_last_slash($ENV{"CVSROOT"});
my $cvsroot_dir = "";
my $i;
my @file_types= qw(cc h cpp c C java);
my $file_types_string;
my @modules;

# ------------------------------------------------------------
# path where all our files are stored.
# if $cvsdata is not set, use current directory instead.
# ------------------------------------------------------------
if($cvsdata) {
}else{
    print STDERR "WARNING: \$CVSDATA not set!\n";
    exit(0);
}

if ($#ARGV < 0) {
    usage();
}

$i = 0;
while ($i<=$#ARGV) {
    if (0) {
    } elsif ($ARGV[$i] eq "-d") {
        $i++;
        $cvsroot = strip_last_slash($ARGV[$i]);
        $i++;
    } elsif ($ARGV[$i] eq "-h") {
        usage();
    } else {
        $i++;
    }
}

if ($#ARGV < 0) {
    usage();
}

if (read_root_dir()) {
    $_ = $ARGV[0];
    $ARGV[0]=~tr/\//\_/;
    $ARGV[0] = "$cvsdata/$cvsroot_dir/db/$ARGV[0]/$ARGV[0].db";
} else {
    die "cannot find the root specified";
}

# ------------------------------------------------------------
# call cvsmap-script with the same parameters
# ------------------------------------------------------------
system ("cvsquery @ARGV");

sub read_root_dir {
    open(CVSROOTS, "<$cvsdata/CVSROOTS");
    my $j = 0;
    my $val = 0;
    while(<CVSROOTS>) {
        chomp;
        my @fields = split(/\ /);
        if (strip_last_slash($fields[0]) eq $cvsroot) {
            $cvsroot_dir = $fields[1];
            $val = 1;
            last;
        }
        $j++;
    }
    close(CVSROOTS);
    undef $j;
    return $val;
}

sub usage()
{
print << "EOF";
cvsquery-script 0.1 (2001-2-26)
Usage: cvsquery-script package [Options] [Options] ...

Options:
  -h                     print out this message
  -d CVSROOT             specify the \$CVSROOT variable.
                         if this flag is not used. default \$CVSROOT is used.
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
