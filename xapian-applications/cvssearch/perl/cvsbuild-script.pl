use strict;
use Cvssearch;

sub usage() {
    print << "EOF";
cvsbuilddb 1.0 (2001-2-22)
Usage $0 [Options]
        
Options:
  -d CVSROOT           specify the \$CVSROOT variable.
                       if this flag is not used. default \$CVSROOT is used.
  -t file_types        specify file types of interest. e.g. -t "html java"
                       will only do the line mapping for files with extension
                       .html and .java; default types include: c cc cpp C h.
  modules              a list of modules to built, e.g. koffice/kword  kdebase/konqueror
  -comp                simulates the comparison between the two alignment implementations,
                       no databases will be written.
  -f app.list          a file containing a list of modules
  -h                   print out this message
EOF
exit 0;
}

# ------------------------------------------------------------
# check for existence of programs used in this script
# if not found, exit.
# ------------------------------------------------------------
my $pwd = `pwd`;
chomp $pwd;

my $cvsindex = "$pwd/cvsindex";
my $cvsmap = "$pwd/cvsmap";
my $cvs2cl = "$pwd/cvs2cl";

if (not (-x $cvsindex)) {
    print STDERR "WARNING: a program used in this script called cvsindex is not found.\n";
    print STDERR "please change to the directory where $0 is stored and execute the script again.\n";
    print STDERR "(cvsindex should be in the same directory)\n";
    exit(1);
}

if (not (-x $cvsmap)) {
    print STDERR "WARNING: a program used in this script $cvsmap is not found.\n";
    print STDERR "please change to the directory where $0 is stored and execute the script again.\n";
    print STDERR "(cvsmap should be in the same directory)\n";
    exit(1);
}

my $mask = umask "0002";
my $comp_mode = 0;
my $cvsdata = Cvssearch::get_cvsdata();
my $cvsroot = $ENV{"CVSROOT"};
my $cvscache = "cache";
my @file_types= qw(cc h cpp c C java);
my $file_types_string;
my @modules;

$cvsroot = Cvssearch::strip_last_slash($cvsroot);

# ------------------------------------------------------------
# tried to get $CVSDATA and failed
# warn user about it and exit
# ------------------------------------------------------------
if ($cvsdata eq "") {
    print STDERR "WARNING: an environment variable \$CVSDATA\n";
    print STDERR "is not set, nor can the value be read from\n";
    print STDERR "a configuration file.\n";
    print STDERR "please export the environment variable\n";
    print STDERR "\$CVSDATA and try again\n";
    exit(1);
}

# ------------------------------------------------------------
# check if user indeed wants to do something
# ------------------------------------------------------------
if ($#ARGV < 0) {
    usage();
}

# ------------------------------------------------------------
# path where all our files are stored.
# if not there, create it.
# ------------------------------------------------------------
if(($cvsdata ne "") && (not (-d $cvsdata))) {
    # ----------------------------------------
    # cause the umask is set to 002
    # the directory should be publically readable
    # and executable
    # ----------------------------------------
    mkdir ("$cvsdata",0777) || die "cannot mkdir $cvsdata: $!";
}

# ------------------------------------------------------------
# path where all temp files are created during query time.
# ------------------------------------------------------------
if(($cvscache ne "") && (not (-d $cvscache))) {
    mkdir ("$cvscache", 0777) || die "cannot mmkdir $cvscache: $!";
    system ("chmod o+rwx $cvscache");
}


# ----------------------------------------
# dump the $CVSDATA variable to a file
# ----------------------------------------
open (CVSSEARCHCONF, ">cvssearch.conf") || die "cannot create cvssearch.conf file: $!";
print CVSSEARCHCONF "CVSDATA $cvsdata\n";
close CVSSEARCHCONF;

# ----------------------------------------
# parse command line arguments
# ----------------------------------------
my $i = 0;
while ($i<=$#ARGV) {
    if (0) {
    } elsif ($ARGV[$i] eq "-d") {
        # ----------------------------------------
        # expect the next variable to be new 
        # the repository
        # ----------------------------------------
        $i++;
        $cvsroot = &Cvssearch::strip_last_slash($ARGV[$i]);
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
    } elsif ($ARGV[$i] eq "-comp") {
        $comp_mode = 1;
        $i++;
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
}

# ----------------------------------------
# uses @modules and $cvsroot $cvsdata 
# variables
# ----------------------------------------
cvsbuild();

sub cvsbuild {
    my $list_file="$cvsdata/.list";

    my $root =&Cvssearch::read_root_dir($cvsroot, $cvsdata);
    
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
        my $app_path = &Cvssearch::strip_last_slash ($_);
        my $app_name = $app_path; 
       
        $app_name =~ tr!/!_!;
        
        if ($app_path ne "" ) {
            # ----------------------------------------
            # checkout files
            # special case if the package is .
            # meaning all packages under the repository
            # ----------------------------------------
            my $checkout_start_date = time;
            if ($app_path eq ".") {
                if (chdir ("$cvsdata/$root/src")) {
                    system ("cvs -l -d $cvsroot checkout . 2>/dev/null");
                    chdir ($pwd);
                }
            } else {
                if (chdir ("$cvsdata/$root/src")) {
                    system ("cvs -l -d $cvsroot checkout -N $app_path 2>/dev/null"); 
                    chdir ($pwd);
                }
            }
            my $checkout_end_date = time;
            
            # ----------------------------------------
            # find files
            # ----------------------------------------
            my $find_start_date = time;
            my $found_files = 0;
            my @files;

            if (chdir ("$cvsdata/$root/src/$app_path")) {
                system ("chmod -R o+r *");
                for ($i = 0; $i <= $#file_types; ++$i) {
                    open(FIND_RESULT, "find . -name \"*.$file_types[$i]\"|");
                    while (<FIND_RESULT>) {
                        chomp;
                        my $length = length($_);
                        @files = (substr($_, 2, $length-2), @files);
                        $found_files = 1;
                    }
                    close(FIND_RESULT);
                }
                chdir ($pwd);
            }
            my @sorted_files = sort @files;
            open(LIST, ">$list_file") || die "cannot create temporary file list\n";
            foreach (@sorted_files) {
                print LIST "$_\n";
            }
            close(LIST);
            my $find_end_date = time;
            
            my $map_start_date;
            my $map_end_date;

            my $index_start_date;
            my $index_end_date;
            # ----------------------------------------
            # do cvsmap and cvsindex
            # ----------------------------------------
            if ($found_files) {
                my $prefix_path = "$cvsdata/$root/db/$app_name";
                if ($comp_mode) {
                    if (chdir ("$cvsdata/$root/src/$app_path")) {
                        system ("$cvsmap -d $cvsroot".
                                " -i $list_file".
                                " -comp");
                        chdir ($pwd);
                    }
                } else {
                    $map_start_date = time;
                    Cvssearch::cvsupdatedb ($root, "-r", $app_name);
                    if (chdir ("$cvsdata/$root/src/$app_path")) {
                        system ("$cvsmap -d $cvsroot".
                                " -i $list_file".
                                " -db $prefix_path.db".
                                " -st $prefix_path.st".
                                " -f1 $prefix_path.cmt".
                                " -f2 $prefix_path.offset".
                                " -m  $app_path".
                                " -cl $cvs2cl");
                        chdir ($pwd);
                    }
                    $map_end_date = time;
                    $index_start_date =time;
                    system ("$cvsindex $root:$app_name");
                    $index_end_date =time;
                    Cvssearch::cvsupdatedb ($root, "-i", $app_name);
                    # ----------------------------------------
                    # clear db directory
                    # ----------------------------------------
                    if (-d "$prefix_path.db") {
                        system ("rm -rf $prefix_path.db/*");
                    } else {
                        mkdir ("$prefix_path.db",0777) || die " cannot mkdir $prefix_path.db: $!";
                    }
                    system ("mv $prefix_path.db? $prefix_path.db");
                    system ("chmod o+r $prefix_path.offset");
                    system ("chmod o+rx $prefix_path.db");
                    system ("chmod o+rx $prefix_path.om");
                    system ("chmod o+r $prefix_path.db/*");
                    system ("chmod o+r $prefix_path.om/*");

                    my $berkeley_size = 0;
                    my $xapian_size = 0;
                    my $cmt_size = 0;
                    
                    open(SIZE, "du -k $prefix_path.db|");
                    while (<SIZE>) {
                        chomp;
                        my @fields = split(/\t/);
                        $berkeley_size = $fields[0];
                        last;
                    }
                    close (SIZE);

                    open(SIZE, "du -k $prefix_path.om|");
                    while (<SIZE>) {
                        chomp;
                        my @fields = split(/\t/);
                        $xapian_size = $fields[0];
                        last;
                    }
                    close (SIZE);

                    # unlink "$prefix_path.cmt";

                    my $code_words = 0;

                    my $pwd = `pwd`;
                    chomp $pwd;
                    my ($entries, $authors, $cvs_words) = 
                      Cvssearch::cvs_stat ($pwd, "$cvsdata/$root/src", $app_path);
                    
                    if (chdir ("$cvsdata/$root/src/$app_path")) {
                        open(LIST, "<$list_file") || die "cannot read from  $list_file: $!\n";
                        while (<LIST>) {
                            chomp;
                            $code_words += Cvssearch::code_comment_counter ($_);
                        }
                        close(LIST);
                        chdir ("$pwd");
                    }
                    
                    open(STAT, ">>$prefix_path.st") || die "cannot append to statistics file\n";
                    print STAT "total   # words of code comment :\t$code_words words\n";
                    print STAT "total   # words of cvs  comment :\t$cvs_words words\n";
                    print STAT "total   # of revision commits   :\t$entries\n";
                    print STAT "total   # of authors            :\t$authors\n";
                    print STAT "\n";
                    print STAT "total build time               :\t"
                        . (time - $checkout_start_date) . " seconds\n";
                    print STAT "   checkout time               :\t"
                        . ($checkout_end_date - $checkout_start_date) . " seconds\n";
                    print STAT "   map      time               :\t"
                        . ($map_end_date      - $map_start_date). " seconds\n";
                    print STAT "   index    time               :\t"
                        . ($index_end_date    - $index_start_date). " seconds\n";
                    print STAT "\n";
                    print STAT "berkeley database size         :\t"
                        . $berkeley_size. "\tkb at $prefix_path.db\n";
                    print STAT "xapian   database size         :\t"
                        . $xapian_size  . "\tkb at $prefix_path.om\n";
                    close(STAT);
                }

            }
        }
    }
}

