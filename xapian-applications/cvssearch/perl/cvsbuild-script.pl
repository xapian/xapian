use strict;
use cvssearch;

# ------------------------------------------------------------
# check for existence of programs used in this script
# if not found, exit.
# ------------------------------------------------------------
my $cvsindex = "./cvsindex";
my $cvsupdatedb = "./cvsupdatedb";
my $cvsmap = "./cvsmap";

if (not (-x $cvsindex)) {
    print STDERR "Warning: a program used in this script cvsindex is not found.\n";
    print STDERR "please change to the directory where $0 is stored and execute the script again.\n";
    print STDERR "(cvsindex should be in the same directory)\n";
    exit(1);
}

if (not (-x $cvsupdatedb)) {
    print STDERR "Warning: a program used in this script $cvsupdatedb is not found.\n";
    print STDERR "please change to the directory where $0 is stored and execute the script again.\n";
    print STDERR "(cvsupdatedb should be in the same directory)\n";
    exit(1);
}

if (not (-x $cvsmap)) {
    print STDERR "Warning: a program used in this script $cvsmap is not found.\n";
    print STDERR "please change to the directory where $0 is stored and execute the script again.\n";
    print STDERR "(cvsmap should be in the same directory)\n";
    exit(1);
}

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
    exit(1);    
}

# ----------------------------------------
# uses @modules and $cvsroot $cvsdata 
# variables
# ----------------------------------------
cvsbuild();

sub cvsbuild {
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
                    system ("$cvsmap -d $cvsroot".
                            " -i $list_file".
                            " -comp");
                } else {
                    $map_start_date = time;
                    system ("$cvsupdatedb $root -r $app_name");
                    system ("$cvsmap -d $cvsroot".
                            " -i $list_file".
                            " -db $prefix_path.db".
                            " -st $prefix_path.st".
                            " -f1 $prefix_path.cmt".
                            " -f2 $prefix_path.offset");
                    $map_end_date = time;
                    $index_start_date =time;
                    system ("$cvsindex $root:$app_name");
                    $index_end_date =time;
                    system ("$cvsupdatedb $root $app_name");
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
                    my $omsee_size = 0;
                    my $cmt_size = 0;
                    
                    open(SIZE, "du $prefix_path.db|");
                    while (<SIZE>) {
                        chomp;
                        my @fields = split(/\t/);
                        $berkeley_size = $fields[0];
                        last;
                    }
                    close (SIZE);

                    open(SIZE, "du $prefix_path.om|");
                    while (<SIZE>) {
                        chomp;
                        my @fields = split(/\t/);
                        $omsee_size = $fields[0];
                        last;
                    }
                    close (SIZE);

                    open(SIZE, "du $prefix_path.cmt|");
                    while (<SIZE>) {
                        chomp;
                        my @fields = split(/\t/);
                        $cmt_size = $fields[0];
                        last;
                    }
                    close(SIZE);

                    my $cvs_words;
                    my $code_words = 0;

                    my $pwd = `pwd`;
                    chomp $pwd;
                    $cvs_words = `./cvs_comment_extractor $pwd $cvsdata/$root/src $app_path|wc -c`;
                    chomp($cvs_words);
                    $cvs_words = (0 + $cvs_words);

                    open(LIST, "<$list_file") || die "cannot create temporary file list\n";
                    while (<LIST>) {
                          chomp;
                          my $output = `./code_comment_extractor $_|wc -c`;
	                  chomp($output);
                          $code_words += (0 + $output);
                    }
                    close(LIST);

                    open(STAT, ">>$prefix_path.st") || die "cannot append to statistics file\n";
                    print STAT "total   # words of code comment :\t$code_words words\n";
                    print STAT "total   # words of cvs  comment :\t$cvs_words words\n";
                    print STAT "\n";
                    print STAT "total build time               :\t". (time - $checkout_start_date) . " seconds\n";
                    print STAT "   checkout time               :\t". ($checkout_end_date - $checkout_start_date) . " seconds\n";
                    print STAT "   map      time               :\t". ($map_end_date      - $map_start_date). " seconds\n";
                    print STAT "   index    time               :\t". ($index_end_date    - $index_start_date). " seconds\n";
                    print STAT "\n";
                    print STAT "berkeley database size         :\t". $berkeley_size. "\tkb at $prefix_path.db\n";
                    print STAT "omsee    database size         :\t". $omsee_size  . "\tkb at $prefix_path.om\n";
                    print STAT "cmt      file     size         :\t". $cmt_size . "\tkb at $prefix_path.cmt\n";
                    close(STAT);
                }

            }
        }
    }
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
  -comp                simulates the comparison between the two alignment implementations,
                       no databases will be written.
  -f app.list          a file containing a list of modules
  -h                   print out this message
EOF
exit 0;
}

