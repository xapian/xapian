package Cvssearch;

use strict;

# ----------------------------------------
# symbols to export by default
# ----------------------------------------
my @EXPORT = qw(read_cvsroot_dir, read_root_dir, strip_last_slash, get_cvsdata, get_cvs_stat, code_comment_counter);



sub get_cvsdata {
    # ----------------------------------------
    # see if $CVSDATA is set
    # ----------------------------------------
    my $found = 0;
    my $cvsdata;

    $cvsdata = $ENV{"CVSDATA"};

    if (0) {
    } elsif ($cvsdata ne "") {
        # ----------------------------------------
        # environment is set
        # ----------------------------------------
    } elsif (-e "cvssearch.conf") {
        open (CVSSEARCHCONF, "<cvssearch.conf") || die "cannot read from cvssearch.conf: $!";
        while (<CVSSEARCHCONF>) {
            chomp;
            my @fields = split(/\ /);
            if ($fields[0] eq "CVSDATA") {
                $cvsdata = $fields[1];
                $ENV{"CVSDATA"} = $cvsdata;
            }
        }
        close (CVSSEARCHCONF);
    }
    return $cvsdata;
}

sub mk_root_dir {
    my ($root_dir, $cvsdata) = @_;
    mkdir ("$cvsdata/$root_dir",    0777)|| die " cannot create directory $cvsdata/$root_dir";
    mkdir ("$cvsdata/$root_dir/db", 0777)|| die " cannot create directory $cvsdata/$root_dir/db";
    mkdir ("$cvsdata/$root_dir/src",0777)|| die " cannot create directory $cvsdata/$root_dir/src";
}

sub read_cvsroot_dir {
    my ($root_dir, $cvsdata) = @_;
    my $cvsroot;
    # ----------------------------------------
    # read root entries
    # ----------------------------------------
    open(CVSROOTS, "<$cvsdata/CVSROOTS");
    while(<CVSROOTS>) {
        chomp;
        my @fields = split(/\ /);
        if ($fields[1] eq $root_dir) {
            # ----------------------------------------
            # found one
            # ----------------------------------------
            $cvsroot = $fields[0];
            last;
        }
    }
    close(CVSROOTS);
    return $cvsroot;
}

sub read_root_dir {
    my ($cvsroot, $cvsdata) = @_;
    my $root_dir;
    if ($cvsroot) {
        # ----------------------------------------
        # read root entries
        # ----------------------------------------
        open(CVSROOTS, "<$cvsdata/CVSROOTS");
        my $j = 0;
        while(<CVSROOTS>) {
            chomp;
            my @fields = split(/\ /);
            if (strip_last_slash($fields[0]) eq $cvsroot) {
                # ----------------------------------------
                # found one
                # ----------------------------------------
                $root_dir = $fields[1];
                last;
            }
            $j++;
        }
        close(CVSROOTS);

        # ----------------------------------------
        # lets try to create that directory if not
        # previously existed
        # ----------------------------------------
        if ($root_dir) {
            if (-d "$cvsdata/$root_dir") {
            } else {
                if (-e "$cvsdata/$root_dir") {
                    unlink ("$cvsdata/$root_dir");
                }
                mk_root_dir($root_dir, $cvsdata);
            }
        } else {
            # ----------------------------------------
            # find a suitable name
            # ----------------------------------------
            while (-e "$cvsdata/root$j") {
                $j++;
            }
            # ----------------------------------------
            # $cvsdata/root$j is not a dir/file entry
            # ----------------------------------------
            $root_dir = "root$j";
            mk_root_dir($root_dir, $cvsdata);

            # ----------------------------------------
            # write to CVSROOTS
            # ----------------------------------------
            open(CVSROOTS, ">>$cvsdata/CVSROOTS") || die "cannot write to $cvsdata/CVSROOTS";
            print CVSROOTS "$cvsroot $root_dir\n";
            close(CVSROOTS);
        }
        undef $j;
    } else {
        die "\$CVSROOT is not specified and -d flag is not used.";
    }
    return $root_dir;
}

sub strip_last_slash {
    my ($path) = @_;
    my $path_length = length($path);
    my $slash = rindex($path, "/");
    if ($slash == $path_length-1) {
        return substr($path, 0, $path_length-1);
    } else {
        return $path;
    }
}


#-------------------------------------------
# encode given string into url
#-------------------------------------------
sub encode{
	my ($string) = @_;
	$string =~ s/ /+/g, $string;
	$string =~ s/([^a-zA-Z0-9])/'%'.unpack("H*",$1)/eg;
	return $string;
}

sub decode{
	my ($string) = @_;
	$string =~ tr/+/ /; #pluses become spaces
	$string =~ s/%(..)/pack('c',hex($1))/ge;
	return $string;
}

#----------------------------------------------
# given name of dump file and fileid
# returns 3 lines of query info followed by
# relevent sections to that fileid
#----------------------------------------------
sub findfile{
	my($d, $id) = @_;
	my $cache = "cache";
	my $dump = `cat $cache/$d`;
	my $ctrlA = chr(01);
	my @dump = split /$ctrlA/, $dump;
	
	my $query = shift @dump;
	foreach (@dump){
		if(/^$id\n/){
			return $query.$_;
		}
	}
}

# ----------------------------------------
# returns a string denoting the weight.
#
# val == max_val returns 
# the color string correspond to max_color.
# val == 0 returns 
# the color string correspond to min_color.
# ----------------------------------------
sub get_color {
    my ($val, $max_val) = @_;
    my $ratio = $val/$max_val;

    my @hex_digit = qw(0 1 2 3 4 5 6 7 8 9 A B C D E F);
  
    # ----------------------------------------
    # change color here.. darkest color
    # ----------------------------------------
    my $max_color_red   = 255;
    my $max_color_blue  = 0;
    my $max_color_green = 232;

    # ----------------------------------------
    # change color here.. lightest color
    # ----------------------------------------
    my $min_color_red   = 255;
    my $min_color_blue  = 255;
    my $min_color_green = 255;
    
    my $distance_red   = $max_color_red   - $min_color_red;
    my $distance_blue  = $max_color_blue  - $min_color_blue;
    my $distance_green = $max_color_green - $min_color_green;


    my $r = $min_color_red  + $ratio * $distance_red;
    my $g = $min_color_green+ $ratio * $distance_green;
    my $b = $min_color_blue + $ratio * $distance_blue;
    
    my $output = "#".
      "$hex_digit[$r/16]$hex_digit[$r%16]".
      "$hex_digit[$g/16]$hex_digit[$g%16]".
      "$hex_digit[$b/16]$hex_digit[$b%16]";
    return $output;
}

sub cvs_stat {
    my ($pwd, $path, $pkg) = @_;
    my $get_msg = 0;
    my $entries;
    my %authors;
    my @authors;
    my $word_count;

    if ($pkg ne "") {
        chdir $path || die "cannot change directory to $path: $!";
        open (ChangeLog, "$pwd/cvs2cl --stdout --xml $pkg 2>/dev/null|");
        while (<ChangeLog>){ 
            chomp;
            my $line = $_;
            if (0) {
            } elsif (/<msg>(.*)<\/msg>/) {
                my @words = split(/\s/, $1);
                $word_count += $#words;
            } elsif (/<msg>/) {
                $get_msg = 1;
            } elsif (/<\/msg>/) {
                $get_msg = 0;
            } elsif ($get_msg) {
                my @words = split(/\s/);
                $word_count += $#words;
            } elsif (m/<entry>/) {
                $entries++;
            } elsif (m/<author>(.*)<\/author>/) {
                $authors{$1}= 1;
            }
        }
        
        close (ChangeLog);
        chdir $pwd || die "cannot change directory to $pwd: $!";
        @authors = keys %authors;
    }
    return ($entries, $#authors, $word_count);
}

sub code_comment_counter {
    my $line;
    my $temp;
    my $word_count;

    my ($file) = @_;
    open (FILE, $file);
    while (<FILE>) {
        chomp;
        $line = $_;
        if (m/\/\/(.*)/) {
            my @words = split(/\s/, $1);
            $word_count += $#words;
        }
        $temp .= $line;
        $temp .= " ";
    }
    close(FILE);
    while ($temp =~ s#(/\*(.*?)\*/)##) {
           my @words = split(/\s/, $1);
           $word_count += $#words;
       }
    return $word_count;
}

#-----------------------------------------------------------------------
# A script to update a database content file which contains all 
# the database that has been built.
# This is used to keep track of which database has been built
# Usage:
# 1. cvsupdatedb root filepath
#    inserts a database stored in the filepath under root if the database 
#    is not already stored.
#    e.g. cvsupdatedb -i kdebase/konqueror
# 2. cvsupdatedb root -r filepath
#    remove a database stored in the filepath
# 3. cvsupdatedb root -f filepath
#    finds if the database for this filepath is built, if not, 
#    returns all the database built under that filepath if any.
# 
# Author: Annie Chen - anniec@cse.unsw.edu.au
# Date: Feb 17 2001
#------------------------------------------------------------------------

sub cvsupdatedb {
    my ($root, $flag, $filepath) = @_;
    my $cvsdata = get_cvsdata(); # path where database content file is stored
    my $filename = "dbcontent"; # file containing database built
    my $path;
    if ($cvsdata eq ""){
        print STDERR "Warning: \$CVSDATA is not set and cvssearch.conf cannot be read.";
        $path = "$cvsdata/$root/$filename";
    }
    
    if(-d "$cvsdata/$root") {
        if($flag eq "-r"){ # remove database
            my @files;
            if (-e $path) {
                # ----------------------------------------
                # read current content
                # ----------------------------------------
                open FILE, "<$path";
                @files = <FILE>;
                close FILE;
                chomp @files;
            }
            open FILE, ">$path";
            foreach (@files){
                if($filepath eq $_){
                    print "... $filepath found and deleted.\n";
                } else {
                    print FILE "$_\n";
                }
            }
            close FILE;
        } elsif($flag eq "-f"){ # find database
            my @bestmatches;
            
            if (-e $path) {
                @bestmatches = `grep $filepath\$ $path`; # match filepath from the end
            }
            
            if(@bestmatches){
                print @bestmatches;
            }else{ #find everything below it
                if ($filepath eq ".") {
                    # ----------------------------------------
                    # whole repository
                    # ----------------------------------------
                    if (-e $path) {
                        open (FILE, "<$path");
                        while (<FILE>) {
                            print $_;
                        }
                        close FILE;
                    }
                }else {
                    my @secmatches;
                    if (-e $path) {
                        @secmatches = `grep $filepath $path`;
                    }
                    print @secmatches;
                }
            }
        } elsif($flag eq "-i"){
            # ----------------------------------------
            # insert database
            # ----------------------------------------
            if(-e $path) {
                if(!`grep ^"$filepath"\$ $path`){
                    print "... $filepath inserted.\n";
                    open (PATH, ">>$path");
                    print PATH "$filepath\n";
                    close PATH;
                } else {
                    print "... $filepath already exists!\n";
                }
            } else {
                open (PATH, ">$path");
                print PATH "$filepath\n";
                close PATH;
                system ("chmod o+r $path");
            }
        }
    }
    else {
        print STDERR "Warning: \$root $root does not exist.\n";
        exit(1);
    }
}
return 1;
    
    
