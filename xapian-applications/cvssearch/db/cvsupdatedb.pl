#-----------------------------------------------------------------------
# A script to update a database content file which contains all 
# the database that has been built.
# This is used to keep track of which database has been built
# Usage:
# 1. ./cvsupdatedb.pl filepath
#    inserts a database stored in the filepath if the database 
#    is not already stored.
#    e.g. ./insertdb.pl kdebase/konqueror
# 2. ./cvsupdatedb.pl -d filepath
#    deletes a database stored in the filepath
# 3. ./cvsupdatedb.pl -f filepath
#    finds if the database for this filepath is built, if not, 
#    returns all the database built under that filepath if any.
# 
# Author: Annie Chen - anniec@cse.unsw.edu.au
# Date: Feb 17 2001
#------------------------------------------------------------------------

#$ENV{'CVSDATA'} = '/home/annie/codeweb/web'; # for testing, to be deleted

$CVSDATA = $ENV{"CVSDATA"}; # path where database content file is stored
$filename = "dbcontent"; # file containing database built

if($CVSDATA){
	$path = "$CVSDATA/$filename";
}else{
	print STDERR "WARNING: \$CVSDATA not set!\n";
	$path = $filename;
}

if($ARGV[0] eq "-d"){ # delete database
	@files = `cat $path`;
	open FILE, ">$path";
	foreach (@files){
		if("$ARGV[1]\n" ne $_){
			print FILE $_;
		}else{
			print "$ARGV[1] found and deleted.\n";
		}
	}
	close FILE;
}elsif($ARGV[0] eq "-f"){ # file database
	@matches = `grep ^$ARGV[1] $path`;
	foreach (@matches){
		if("$ARGV[1]\n" eq $_){
			print $ARGV[1];
			exit(0);
		}
	}
	print @matches;
	
}else{ # insert database
	if(!`grep ^"$ARGV[0]"\$ $path`){
		print "$ARGV[0] inserted.";
		`echo "$ARGV[0]" >> $path`;
	}else{
		print "$ARGV[0] already exists!\n";
	}
	
}
