# -* perl *-

use strict;
use cvssearch;

my $mask = umask "0002";
my $cvsdata = $ENV{"CVSDATA"};
my $cvsroot = $ENV{"CVSROOT"};
$cvsdata = &cvssearch::get_cvsdata();
$cvsroot = &cvssearch::strip_last_slash($cvsroot);

my @file_types= qw(cc h cpp c C java);
my $file_types_string;
my @modules;

# ------------------------------------------------------------
# path where all our files are stored.
# ------------------------------------------------------------
if($cvsdata eq "") {
    print STDERR "WARNING: \$CVSDATA not set!\n";
    exit(1);
}

if ($#ARGV < 0) {
    usage();
}

my $i = 0;

# ----------------------------------------
# parse command line arguments
# ----------------------------------------
while ($i<=$#ARGV) {
    if (0) {
    } elsif ($ARGV[$i] eq "-d") {
        # ----------------------------------------
        # expect the next variable to be new 
        # the repository
        # ----------------------------------------
        $i++;
        $cvsroot = &cvssearch::strip_last_slash($ARGV[$i]);
        $i++;
    } elsif ($ARGV[$i] eq "-t") {
        # ----------------------------------------
        # specify different extensions to parse
        # ----------------------------------------
        $i++;
        if ($i<=$#ARGV) {
            $file_types_string = $ARGV[$i];
            @file_types = split(/\" /, $ARGV[$i]);
            $i++;
        }
    } elsif ($ARGV[$i] eq "-f") {
        # ----------------------------------------
        # specify the file containing all the 
        # packages.
        # ----------------------------------------
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
        # ----------------------------------------
        # assume they are package names
        # ----------------------------------------
        @modules = (@modules, $ARGV[$i]);
        $i++;
    }
}

if ($cvsroot eq "") {
    # ----------------------------------------
    # $CVSROOT is not set and user didn't
    # specify a repository location.
    # ----------------------------------------
    print STDERR "WARNING: \$CVSROOT not set or not specified using -d flag!\n";
    usage();
    exit(1);    
}

# ----------------------------------------
# uses @modules and $cvsroot $cvsdata 
# variables
# ----------------------------------------
cvsbuild();

sub cvsbuild {
    my $list_file="$cvsdata/.list";
    my $time_file="$cvsdata/.time";

    my $root =&cvssearch::read_root_dir($cvsroot, $cvsdata);
    my $delta_time = 0;
    
    # ----------------------------------------
    # clear temp files
    # ----------------------------------------
    unlink $time_file;
    unlink $list_file;
    
    open(TIME, ">$time_file");
    
    foreach (@modules) {

        # ----------------------------------------
        # e.g. 
        # $app_path=kdebase/konqueror without
        # slash at the end
        # #app_name=kdebase_konqueror where all 
        # /=_.
        # ----------------------------------------
        my $app_path = &cvssearch::strip_last_slash ($_);
        my $app_name = $app_path; 
       
        $app_name =~tr/\//\_/;
        
        if ($app_path ne "" ) {
            # ----------------------------------------
            # checkout files
            # special case if the package is .
            # meaning all packages under the repository
            # ----------------------------------------
            if ($app_path eq ".") {
                system ("cvs -l -d $cvsroot checkout -d $cvsdata/$root/src . 2>/dev/null");
            } else {
                system ("cvs -l -d $cvsroot checkout -d $cvsdata/$root/src -N $app_path 2>/dev/null"); 
            }
            
            # ----------------------------------------
            # find files
            # ----------------------------------------
            my $found_files = 0;
            open(LIST, ">$list_file") || die "cannot create temporary file list\n";
            for ($i = 0; $i <= $#file_types; ++$i) {
                open(FIND_RESULT, "find $cvsdata/$root/src/$app_path -name \"*.$file_types[$i]\"|");
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
                print TIME "$cvsdata/$root/src/$app_path", "\n";
                print TIME "Started  @ ", `date`;
                my $start_date = time;
                my $prefix_path = "$cvsdata/$root/db/$app_name";
                system ("./cvsmap -d $cvsroot".
                        " -i $list_file".
                        " -db $prefix_path.db".
                        " -f1 $prefix_path.cmt".
                        " -f2 $prefix_path.offset");
                
                system ("./cvsindex $root:$app_name");
                system ("./cvsupdatedb $root $app_name");

                # ----------------------------------------
                # clear db directory
                # ----------------------------------------
                if (-d "$prefix_path.db") {
                    system ("rm -rf $prefix_path.db/*");
                } else {
                    mkdir ("$prefix_path.db",0777) || die " cannot mkdir $prefix_path.db: $!";
                }
                system ("mv $prefix_path.db? $prefix_path.db");
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

