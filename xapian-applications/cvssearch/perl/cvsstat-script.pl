use strict;
use cvssearch;

my $mask = umask "0002";
my $comp_mode = 0;
my $cvsdata = &cvssearch::get_cvsdata();
my $cvsroot = &cvssearch::strip_last_slash($ENV{"CVSROOT"});

my @file_types= qw(cc h cpp c C java);
my $file_types_string;
my @modules;


# ------------------------------------------------------------
# path where all our files are stored.
# ------------------------------------------------------------
if($cvsdata eq "") {
} elsif (not (-d $cvsdata)) {
    # ----------------------------------------
    # cause the umask is set to 002
    # the directory should be publically readable
    # and executable
    # ----------------------------------------
    mkdir ("$cvsdata",0777) || die " cannot mkdir $cvsdata: $!";
}

if ($#ARGV < 0) {
    usage();
}

my $i = 0;

if (-d $cvsdata) {
} else {
   mkdir ("$cvsdata",0777) || die " cannot mkdir $cvsdata.db: $!";
}

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
cvsstat();

sub cvsstat {
    my $list_file="$cvsdata/.list";

    my $root =&cvssearch::read_root_dir($cvsroot, $cvsdata);
    
    # ----------------------------------------
    # clear temp files
    # ----------------------------------------
    unlink $list_file;
    
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
            my $checkout_start_date = time;
            if ($app_path eq ".") {
                system ("cvs -l -d $cvsroot checkout -d $cvsdata/$root/src . 2>/dev/null");
            } else {
                system ("cvs -l -d $cvsroot checkout -d $cvsdata/$root/src -N $app_path 2>/dev/null"); 
            }
            my $checkout_end_date = time;
            
            # ----------------------------------------
            # find files
            # ----------------------------------------
            my $find_start_date = time;
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
            my $find_end_date = time;

            if ($found_files) {
                my $prefix_path = "$cvsdata/$root/db/$app_name";
                my $cvs_words;
                my $code_words = 0;
                
                my $pwd = `pwd`;
                chomp $pwd;
                my ($entries, $authors, $cvs_words) = &cvssearch::get_cvs_stat($pwd, "$cvsdata/$root/src", $app_path);
                
                open(LIST, "<$list_file") || die "cannot read temporary file $list_file: $!\n";
                while (<LIST>) {
                    chomp;
                    my $output = `./code_comment_extractor $_|wc -c`;
                    chomp($output);
                    $code_words += (0 + $output);
                }
                close(LIST);
                
                open(STAT, ">>$prefix_path.st2") || die "cannot append to statistics file\n";
                print STAT "total   # words of code comment :\t$code_words words\n";
                print STAT "total   # words of cvs  comment :\t$cvs_words words\n";
                print STAT "total   # of revision commits   :\t$entries\n";
                print STAT "total   # of authors            :\t$authors\n";
                print STAT "\n";
                close(STAT);
            }
        }
    }
}

sub usage() {
    print << "EOF";
cvsstat 1.0 (2001-2-22)
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

