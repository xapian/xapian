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
# ------------------------------------------------------------
if($cvsdata) {
}else{
    print STDERR "WARNING: \$CVSDATA not set!\n";
    exit(1);
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

read_root_dir();
cvsbuild();


sub read_root_dir {
    if ($cvsroot) {
        open(CVSROOTS, "<$cvsdata/CVSROOTS");
        my $j = 0;
        while(<CVSROOTS>) {
            chomp;
            my @fields = split(/\ /);
            if (strip_last_slash($fields[0]) eq $cvsroot) {
                $cvsroot_dir = $fields[1];
                last;
            }
            $j++;
        }
        close(CVSROOTS);
        if ($cvsroot_dir) {
            if (-d "$cvsdata/$cvsroot_dir") {
            } else {
                mkdir "$cvsdata/$cvsroot_dir"    || die " cannot create directory $cvsdata/$cvsroot_dir";
                mkdir "$cvsdata/$cvsroot_dir/db" || die " cannot create directory $cvsdata/$cvsroot_dir/db";
                mkdir "$cvsdata/$cvsroot_dir/src"|| die " cannot create directory $cvsdata/$cvsroot_dir/src";
            }
        } else {
            while (-d "$cvsdata/root$j") {
                $j++;
            }
            open(CVSROOTS, ">>$cvsdata/CVSROOTS") || die "cannot write to $cvsdata/CVSROOTS";
            $cvsroot_dir = "root$j";
            print CVSROOTS "$cvsroot $cvsroot_dir\n";
            mkdir "$cvsdata/$cvsroot_dir"    || die " cannot create directory $cvsdata/$cvsroot_dir";
            mkdir "$cvsdata/$cvsroot_dir/db" || die " cannot create directory $cvsdata/$cvsroot_dir/db";
            mkdir "$cvsdata/$cvsroot_dir/src"|| die " cannot create directory $cvsdata/$cvsroot_dir/src";
        }
        undef $j;
    } else {
        die "\$CVSROOT is not specified and -d flag is not used.";
    }
}

sub cvsbuild {
    my $list_file="$cvsdata/.list";
    my $time_file="$cvsdata/.time";
    
    my $delta_time = 0;
    unlink $time_file;
    unlink $list_file;
    
    open(TIME, ">$time_file");
    
    foreach (@modules) {
        my $app_path = $_;
        my $app_name = $app_path;
        
        $app_name =~tr/\//\_/;
        
        print "app_name $app_name app_path $app_path\n";
        if ($app_path ne "" ) {
            # ----------------------------------------
            # checkout files
            # ----------------------------------------
            system ("cvs -d $cvsroot checkout -d $cvsdata/$cvsroot_dir/src -N $app_path 2>/dev/null");
            
            # ----------------------------------------
            # find files
            # ----------------------------------------
            my $found_files = 0;
            open(LIST, ">$list_file") || die "cannot create temporary file list\n";
            for ($i = 0; $i <= $#file_types; ++$i) {
                open(FIND_RESULT, "find $cvsdata/$cvsroot_dir/src/$app_path -name \"*.$file_types[$i]\"|");
                while (<FIND_RESULT>) {
                    $found_files = 1;
                    print LIST $_;
                }
                close(FIND_RESULT);
            }
            close(LIST);
            
            # ----------------------------------------
            # do cvsmap and cvsindex
            # ----------------------------------------
            if ($found_files) {
                print TIME "$cvsdata/$cvsroot_dir/src/$app_path", "\n";
                print TIME "Started  @ ", `date`;
                my $start_date = time;
                
                system ("cvsmap -d $cvsroot".
                        " -i $list_file".
                        " -db $cvsdata/$cvsroot_dir/db/$app_name.db".
                        " -f1 $cvsdata/$cvsroot_dir/db/$app_name.cmt".
                        " -f2 $cvsdata/$cvsroot_dir/db/$app_name.offset");
                
                system ("cvsindex $cvsdata/$cvsroot_dir/db/$app_name.cmt");
                # ----------------------------------------
                # cvsindex should have created directory
                # $cvsdata/$cvsroot_dir/db/$app_name.
                # ----------------------------------------
                system ("mv $cvsdata/$cvsroot_dir/db/$app_name.db* $cvsdata/$cvsroot_dir/db/$app_name");
                print TIME "Finished @ ", `date`;
                $delta_time += time - $start_date;
                print TIME "\n";
            }
        }
    }
    print TIME "Operation Time: $delta_time Seconds \n";
    close(TIME);
}

sub usage() {
    print << "EOF";
cvsbuild 1.0 (2001-2-22)
Usage $0 [Options]
        
Options:
  -d CVSROOT           specify the \$CVSROOT variable.
                       if this flag is not used. default \$CVSROOT is used.
  -t file_types        specify file types of interest. e.g. -t "html java"
                       will only do the line mapping for files with extension
                       .html and .java; default types include: c cc cpp C h.
  modules              a list of modules to built, e.g. koffice/kword  kdebase/konqueror
  -f app.list          a file containing a list of modules
  -h                   print out this message
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
