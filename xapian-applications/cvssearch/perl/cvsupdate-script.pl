#-----------------------------------------------------------------------
# A script to update a database content file which contains all 
# the database that has been built.
# This is used to keep track of which database has been built
# Usage:
# 1. cvsupdatedb root filepath
#    inserts a database stored in the filepath under root if the database 
#    is not already stored.
#    e.g. cvsupdatedb kdebase/konqueror
# 2. cvsupdatedb root -r filepath
#    remove a database stored in the filepath
# 3. cvsupdatedb root -f filepath
#    finds if the database for this filepath is built, if not, 
#    returns all the database built under that filepath if any.
# 
# Author: Annie Chen - anniec@cse.unsw.edu.au
# Date: Feb 17 2001
#------------------------------------------------------------------------

use Cvssearch;

$cvsdata = Cvssearch::get_cvsdata(); # path where database content file is stored
$filename = "dbcontent"; # file containing database built
$root = shift @ARGV;

if($cvsdata){
	$path = "$cvsdata/$root/$filename";
}else{
	print STDERR "WARNING: \$CVSDATA not set!\n";
    exit(1);
}

if(-d "$cvsdata/$root") {
    if($ARGV[0] eq "-r"){ # remove database
        my @files;
        if (-e $path) {
            @files = `cat $path`;
        }
        open FILE, ">$path";
        foreach (@files){
            if("$ARGV[1]\n" ne $_){
                print FILE $_;
            }else{
                print "... $ARGV[1] found and deleted.\n";
            }
        }
        close FILE;
    }elsif($ARGV[0] eq "-f"){ # find database
        my @bestmatches;

        if (-e $path) {
            @bestmatches = `grep $ARGV[1]\$ $path`; # match filepath from the end
        }

        if(@bestmatches){
            print @bestmatches;
        }else{ #find everything below it
            if ($ARGV[1] eq "."){#whole repository
                if (-e $path) {
                    @all = `cat path`;
                    print @all;	
                }
            }else{
                if (-e $path) {
                    @secmatches = `grep $ARGV[1] $path`;
                }
                print @secmatches;
            }
        }
    }else{ # insert database
        if(-e $path) {
            if(!`grep ^"$ARGV[0]"\$ $path`){
                print "... $ARGV[0] inserted.\n";
                open (PATH, ">>$path");
                print PATH "$ARGV[0]\n";
                close PATH;
            }else{
                print "... $ARGV[0] already exists!\n";
            }
        } else {
            open (PATH, ">$path");
            print PATH "$ARGV[0]\n";
            close PATH;
            system ("chmod o+r $path");
        }
    }
} else {
    print STDERR "WARNING: \$root $root does not exist.\n";
    exit(1);
}
