# -* perl *-

@file_types= qw(cc h cpp c C);

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

$list_file="$CVSDATA/.list";
$time_file="$CVSDATA/time";

$delta_time = 0;
unlink $time_file;
unlink $list_file;

open(TIME, ">$time_file");

foreach (@modules) {
    $app_path = $_;
    $app_name = $app_path;
    $app_name =~ tr/\//\_/;
    $app_name = "$CVSDATA/database/$app_name";

    if ($app_path ne "" ) {
        system ("cvs checkout -d $CVSDATA/src -N $app_path 2>/dev/null");
        $found_files = 0;
        open(LIST, ">$list_file") || die "cannot create temporary file list\n";
        for ($i = 0; $i <= $#file_types; ++$i) {
            open(FIND_RESULT, "find $CVSDATA/src/$app_path -name \"*.$file_types[$i]\"|");
            while (<FIND_RESULT>) {
                $found_files = 1;
                print LIST $_;
            }
            close(FIND_RESULT);
        }
        close(LIST);
        
        if ($found_files) {
            print TIME "$app_path", "\n";
            print TIME "Started  @ ", `date`;
            $start_date = time;
            system ("rm -rf $app_name.db*");
            system ("cvsmap -i $list_file -st $app_name.st -db $app_name.db -f1 $app_name.cmt -f2 $app_name.offset");
            print TIME "Finished @ ", `date`;
            $delta_time += time - $start_date;
            print TIME "\n";
        }
    }
}
print TIME "Operation Time: $delta_time Seconds \n";
close(APPS);
close(REC);

sub usage() {
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
