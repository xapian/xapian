package cvssearch;

use strict;

my @EXPORT = qw(read_cvsroot_dir, read_root_dir, strip_last_slash, get_cvsdata);            # symbols to export by default

sub get_cvsdata {
    # ----------------------------------------
    # see if $CVSDATA is set
    # ----------------------------------------
    my $found = 0;
    my $cvsdata;

    $cvsdata = $ENV{"CVSDATA"};

    # ----------------------------------------
    # environment is set
    # try to make sure cvssearch.conf says the
    # same thing.
    # ----------------------------------------
    if ($cvsdata ne "") {
        if (-e "cvssearch.conf") {
            # ----------------------------------------
            # check if there is a conflict 
            # ----------------------------------------
            open (CVSSEARCHCONF, "<cvssearch.conf") || die "cannot read from cvssearch.conf: $!";

            my @fields;
            my $diff = 0;
            while (<CVSSEARCHCONF>) {
                chomp;
                @fields = split(/\ /);
                if ($fields[0] eq "CVSDATA") {
                    $found = 1;
                    if ($cvsdata ne $fields[1]) {
                        $diff = 1;
                        last;
                    }
                }
            }
            close (CVSSEARCHCONF);
            # ----------------------------------------
            # difference found. recreate cvssearch.conf
            # ----------------------------------------
            if ($diff) {
                print STDERR "Warning: cvssearch.conf's CVSDATA : $fields[1]\n";
                print STDERR "            Environment \$CVSDATA : $cvsdata\n";
                print STDERR "         updating cvssearch.conf\n";

                open (CVSSEARCHCONF, ">cvssearch.conf") || die "cannot write to cvssearch.conf: $!";
                print CVSSEARCHCONF "CVSDATA $cvsdata\n";
                close (CVSSEARCHCONF);
                system ("chmod +r cvssearch.conf");
            }
        } else {
            open (CVSSEARCHCONF, ">cvssearch.conf") || die "cannot write to cvssearch.conf: $!";
            print CVSSEARCHCONF "CVSDATA $cvsdata\n";
            close (CVSSEARCHCONF);
            system ("chmod +r cvssearch.conf");
        }
    } else {
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
        } else {
            print STDERR "Warning: cvssearch.conf is not found or cannot be read from the current directory\n";
            print STDERR "         or \$CVSDATA is not set.\n";
            print STDERR "please export an envirnment variable CVSDATA specifying a directory where CVSSearch\n";
            print STDERR "result may be stored\n";
            print STDERR "e.g:";
            print STDERR "export CVSDATA=\"./cvsdata\"";
            exit(1);
        }
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
    if ($root_dir) {
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
    } else {
        die "the \$CVSROOT matches $root_dir is not found in $cvsdata/CVSROOTS";
    }
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

sub strip_last_slash() {
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

return 1;
