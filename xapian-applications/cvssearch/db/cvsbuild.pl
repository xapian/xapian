# -* perl *-

use strict;

my $save_dir;
my $CVSDATA;
my $i;
my @file_types;
my $file_types_string;
my @modules;


# ------------------------------------------------------------
# save our current directory.
# ------------------------------------------------------------
open(PWD, "pwd|");
chomp($save_dir = <PWD>);
close PWD;

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

$i = 0;

while ($i<=$#ARGV) {
    if (0) {
    } elsif ($ARGV[$i] eq "-t") {
        $i++;
        if ($i<=$#ARGV) {
            $file_types_string = $ARGV[$i];
            @file_types = split(/\" /, $ARGV[$i]);
            $i++;
        }
    } elsif ($ARGV[$i] eq "-f") {
        $i++;
        if ($i<=$#ARGV) {
            my $apps_file = $ARGV[$i];
            open(APPS, "<$apps_file");
            chomp(@modules = <APPS>);
            close(APPS);
            $i++;
        }
    } elsif ($ARGV[$i] eq "-h") {
        usage();
    } else {
        @modules = (@modules, $ARGV[$i]);
        $i++;
    }
}


# ------------------------------------------------------------
# call cvsmap-script with the same parameters
# ------------------------------------------------------------
system ("cvsmap-script @ARGV");

# ------------------------------------------------------------
# call cvsindex will all the outputs there
# ------------------------------------------------------------
system ("mkdir $CVSDATA/database");

foreach (@modules) {
    $_ =~ tr/\//\_/;    
    system ("cvsindex $_.cmt");
}

sub usage()
{
print << "EOF";
cvsbuild 0.1 (2001-2-22)
Usage $0 [Options]

Options:
    -t "file_types" specify file types of interest. e.g. -t "html java"
                    will only do the line mapping for files with extension
                    .html and .java; default types include: c cc cpp C h.
    modules         a list of modules to built, e.g. koffice/kword  kdebase/konqueror
    -f app.list     a file containing a list of modules
    -h              print out this message
EOF
exit 0;
}

