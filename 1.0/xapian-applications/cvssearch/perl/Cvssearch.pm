package Cvssearch;

use strict;

# ----------------------------------------
# symbols to export by default
# ----------------------------------------
my @EXPORT = qw(sanitise_root read_cvsroot_dir read_root_dir strip_last_slash get_cvsdata get_cvs_stat code_comment_counter g2l_commit);

# default is optional, but it'll usually be "root0"
sub sanitise_root {
    my ($root, $default) = @_;
    # Default/sanitise input.
    if (!$root || $root !~ /^root\d+$/) {
	$root = $default; 
    }
    return $root;
}

sub get_cvsdata {
    # ----------------------------------------
    # see if $CVSDATA is set
    # ----------------------------------------
    my $found = 0;
    my $cvsdata = "";
    
    if ($ENV{"CVSDATA"}) {
        $cvsdata = $ENV{"CVSDATA"};
    }

    if (! $cvsdata) {
        # ----------------------------------------
        # environment is set
        # ----------------------------------------
        if (-e "cvssearch.conf") {
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
    }
    if (substr($cvsdata, 0, 1) ne "/") {
        # relative path
        my $pwd = `pwd`;
        chomp $pwd;
        $cvsdata = $pwd."/".$cvsdata;
    }
    return $cvsdata;
}

sub mk_root_dir {
    my ($root_dir, $cvsdata) = @_;
    mkdir ("$cvsdata/$root_dir",    0777)|| die " cannot create directory $cvsdata/$root_dir";
    mkdir ("$cvsdata/$root_dir/db", 0777)|| die " cannot create directory $cvsdata/$root_dir/db";
    mkdir ("$cvsdata/$root_dir/src",0777)|| die " cannot create directory $cvsdata/$root_dir/src";
}

sub read_cvsroots {
    my ($cvsdata) = @_;
    my %result;
    open(CVSROOTS, "<$cvsdata/CVSROOTS");
    while(<CVSROOTS>) {
        chomp;
        my @fields = split(/\s/);
        $result{$fields[0]} = $fields[1];
    }
    return %result;
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
        my $j = 0;
        if (-e "$cvsdata/CVSROOTS") {
            open(CVSROOTS, "<$cvsdata/CVSROOTS");
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
        }
        
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
	$string =~ s/ /+/g;
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
    my $output = sprintf("#%2.2X%2.2X%2.2X",$r,$g,$b);
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
    my $line = "";
    my $temp = "";
    my $word_count = 0;
    
    
    my ($file) = @_;
    if (-e "$file") {
        open (MYFILE, "<$file");
        while (<MYFILE>) {
            chomp;
            $line = $_;
            if (m/\/\/(.*)/) {
                my @words = split(/\s/, $1);
                $word_count += $#words;
            }
            $temp .= $line;
            $temp .= " ";
        }
        close(MYFILE);
        while ($temp =~ s#(/\*(.*?)\*/)##) { 
               my @words = split(/\s/, $1);
               $word_count += $#words;
           }
    }
    return $word_count;
}

#-----------------------------------------------------------------------
# A script to update a database content file which contains all 
# the database that has been built.
# This is used to keep track of which database has been built
# Usage:
# 1. cvsupdatedb ($root, "-i", $filepath);
#    inserts a database stored in the filepath under root if the database 
#    is not already stored.
#    e.g. cvsupdatedb ($root, "-i", "kdebase/konqueror");
# 2. cvsupdatedb ($root, "-r", $filepath);
#    remove a database stored in the filepath
# 3. cvsupdatedb ($root, "-f", $filepath);
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
        print STDERR "WARNING: \$CVSDATA is not set and cvssearch.conf cannot be read.";
    }
    $path = "$cvsdata/$root/$filename";
    
    if(-d "$cvsdata/$root") {
        if($flag eq "-r"){      # remove database
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
            my $bestmatches;
            my $output;
            if (-e $path) {
                $bestmatches = `grep '\^$filepath\$\\\|_$filepath\$' $path`; # match filepath from the end
            }
            
            if($bestmatches){
                $output .= $bestmatches;
            }else{              #find everything below it
                if ($filepath eq ".") {
                    # ----------------------------------------
                    # whole repository
                    # ----------------------------------------
                    if (-e $path) {
                        open (FILE, "<$path");
                        while (<FILE>) {
                            $output .= $_;
                        }
                        close FILE;
                    }
                }else {
                    my $secmatches;
                    if (-e $path) {
                        $secmatches = `grep $filepath $path`;
                    }
                    $output .= $secmatches;
                }
            }
            return $output;
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
        print STDERR "WARNING: \$root $root does not exist.\n";
        exit(1);
    }
}

# display a blue header row with 2 columes, first argument on left col, second on right
sub fileheader{
	my ($left, $right) = @_;
	my $header =<<_HTML_;
<p>
<TABLE cellSpacing=0 cellPadding=2 width="100%" border=0>
<TBODY>
<TR>
<TD noWrap bgColor=#3366cc><FONT face=arial,sans-serif color=white 
size=-1>$left&nbsp; </FONT></TD>
<TD noWrap align=right bgColor=#3366cc><FONT face=arial,sans-serif color=white 
size=-1>$right</FONT></TD></TR></TBODY></TABLE>
_HTML_
    return $header;
}

sub getSpectrum{
	my ($num) = @_;
	my $r; my $g; my $b;
	my $curcolor; my @colors; my $i;
	my $curstep; my $range; my $mod;
    if ($num == 0) {
        return;
    }
    
	my $step = (255*6)/$num;
	for($i=0;$i<$num;$i++){
		$curstep = $step*$i;
		$range = int($curstep/255);
		$mod = $curstep%255;
		
		if($range==0){
			# R=255, G=0, B=0->255
			$r=255;
			$g=0;
			$b=$mod;
		}elsif($range==1){
			# R=255->0, G=0, B=255
			$r=255-$mod;
			$g=0;
			$b=255;
		}elsif($range==2){
			# R=0, G=0->255, B=255
			$r=0;
			$g=$mod;
			$b=255;
		}elsif($range==3){
			# R=0, G=255, B=255->0
			$r=0;
			$g=255;
			$b=255-$mod;
		}elsif($range==4){
			# R=0->255, G=255, B=0
			$r=$mod;
			$g=255;
			$b=0
		}else{
			# R=255, G=255->0, B=0
			$r=255;
			$g=255-$mod;
			$b=0;
		}
    	$curcolor = sprintf("#%2.2X%2.2X%2.2X",$r,$g,$b);
        
		push @colors, $curcolor;
	}
	return @colors;
}

### This function will be passed to the standard
### perl sort function for sort the cvs versions.
### ie. 1.10 is later than l.9
sub cmp_cvs_version {
    my $first = $_[0];
    my $second = $_[1];
    
    my @first = split(/\./, $first);
    my @second = split(/\./, $second);
    
    my $size = $#first;
    $size = $#second if ($#first > $#second); 
    
    for (my $i=0; $i<=$size; $i++) {
        if ($first[$i]>$second[$i]) {
            return 1;
        } elsif ($first[$i]<$second[$i]) {
            return -1;
        } else {
            next;
        }
    }
    
    if ($#first > $#second) {
        return 1;
    } elsif ($#first < $#second) {
        return -1;
    } else {
        return 0;
    }
}


sub print_style_sheet {
    my $color    = get_color(50, 100);
    print "<style type=\"text/css\">\n";
    print "body.compare  {background-color:#EEEEEE;}\n";
    print "body {background-color:white;}\n";
# was large not x-lage in CommitSearch
    print "h1 {color:#0066cc; font-size:x-large}\n";
    print "table {background-color:#FFFFFF;}\n";
    print "td    {white-space:pre; overflow:hidden; text-decoration:none;color:black;}\n";
    print ".e {background-color:#ffffff;}\n";
    print ".a {background-color:#CCCCFF;}\n";
    print ".o {background-color:#ccccee;}\n";
    print ".c {background-color:#99FF99;}\n";
# wasn't bold in CommitSearch and PatternMatch
    print ".s {background-color:#3366CC; color:#FFFFFF; font-weight:bold;}\n";
    print ".d {background-color:#FF9999;}\n";
    print ".n {background-color:#EEEEEE;}\n";
    print ".g {background-color:#dddddd;}\n";

    print ".ref {color:#666666;font-size:12;font-family:\"Arial, Helvetica, sans-serif\"}\n";
    print ".popupLink { color: blue; outline: none;}\n";
    print ".popup { position:absolute; visibility:hidden; color:white;background-color:#3366cc;";
    print "layer-background-color:#3366cc;border:2px solid orange; padding: 3px; z-index: 10;}\n";
    print ".red {color:red}\n";
    print ".lightgreen {color:#49C96B}\n";
    print ".lightcvs {color:#00ccff}\n";
    print ".blue {color:#0066cc}\n";
    print ".orange {color:#FF7A12}\n";
    print ".t {background-color:$color;}\n";
    print "</style>\n";
}

sub g2l_commit {
    my ($cvsdata, $root, @commits) = @_;
    my @offsets;
    my @pkgs;
    open (FILE, "<$cvsdata/$root/commit.offset");
    while (<FILE>) {
        chomp;
        my ($pkg, $offset) = split /\s/;
        push (@offsets, $offset);
        push (@pkgs, $pkg);
    }
    close FILE;

    my @results;
    
    foreach (@commits) {
        my $commit = $_;
        my $i = $#offsets / 2;
        my $j = $#offsets / 4;
        
        while (1) {
            if ($j == 0) {
                if ($offsets[$i] > $commit) {
                    push(@results, $pkgs[$i-1]);
                    push(@results, ($commit - $offsets[$i-1]));
                } elsif ($offsets[$i] < $commit) {
                    push(@results, $pkgs[$i]);
                    push(@results, ($commit - $offsets[$i]));
                } elsif ($offsets[$i] == $commit) {
                    push(@results, $pkgs[$i-1]);
                    push(@results, ($commit - $offsets[$i-1]));
                }
                last;
            } elsif ($offsets[$i] > $commit) {
                $i = $i - $j;
                $j = $j / 2;
            } elsif ($offsets[$i] < $commit) {
                $i = $i + $j;
                $j = $j / 2;
            } elsif ($offsets[$i] == $commit) {
                push(@results, $pkgs[$i-1]);
                push(@results, $commit - $offsets[$i-1]);
                last;
            }
        }
    }
    return @results;
}

sub l2g_commit {
}

#-----------------------------------
# highlightquery
# Returns $words with any words matched by $pattern marked up as bold
#-----------------------------------
sub highlightquery {
    my ($words, $pattern) = @_;
    $words =~ s!(^|[^a-zA-Z])($pattern)!$1<b>$2</b>!ig;
    return $words;
}

return 1;
