#! /usr/bin/perl -w
# mkdoc.pl: generate source code structure overview doc from dir_contents files
use strict;

# Declarations
sub get_descriptions();
sub expand_dist_subdirs($);
sub tohtml($);
sub output_html();

# Parse command line parameters
if ($#ARGV < 2 || $#ARGV > 3) {
  print "usage: mkdoc.pl <version> <source directory> <destination> [webroot]\n";
  exit 1;
}

my ($version, $srcdir, $dest, $webroot) = @ARGV;
$webroot = "" unless defined $webroot;

my %descriptions = ();
my %classes = ();

get_descriptions();
output_html();

sub get_descriptions() {
    # Get all the directories in the dist build tree.
    my @dirs = expand_dist_subdirs($srcdir);
    scalar @dirs > 0 or die "DIST_SUBDIRS not found in $srcdir/Makefile.am";

    # Read the contents of any dir_contents's we find.
    my $dir;
    foreach $dir (@dirs) {
	next if $dir =~ m/CVS$/;
	my $contentsfile = "$dir/dir_contents";
	next if ! -r $contentsfile;
	open CONTENTSFILE, $contentsfile or die "Couldn't open $contentsfile ($!)\n";

	my $contents = "";
	while (<CONTENTSFILE>) { $contents .= $_; }
	close CONTENTSFILE;

        # Get directory tag
	if ($contents !~ m#<directory>\s*(.+?)\s*</directory>#is) {
	    print STDERR "Skipping $contentsfile: didn't contain a directory tag\n";
	    next;
	}
	my $directory = $1;
	my $tagdir = "$srcdir/";
	if ($directory ne "ROOT") {
	    $tagdir .= "$directory/";
	}
	$dir = "$dir/";
	$dir =~ s!/(?:\./)+!/!g;
	$tagdir =~ s!/(?:\./)+!/!g;
	if ("$tagdir" ne $dir) {
	    print STDERR "Skipping $contentsfile: incorrect directory tag\n";
	    print STDERR "`$tagdir' != `$dir'\n";
	    next;
	}

        # Get description tag
	if ($contents !~ m#<description>\s*(.+?)\s*</description>#is) {
	    print STDERR "Skipping $contentsfile: didn't contain a description tag\n";
	    next;
	}
	$descriptions{$directory} = $1;
    }
}

sub expand_dist_subdirs($) {
    my $dir = shift;
    my @result;
    open M, "$dir/Makefile.am" or die $!;
    while (<M>) {
	while (s/\\\n$//) { $_ .= <M>; }
	if (s/^\s*DIST_SUBDIRS\s*=\s*//) {
	    # remove trailing whitespace and/or comment
	    s/\s*(?:#.*)?$//;
	    for (split /\s+/) {
		next if $_ eq '.';
		my $d = "$dir/$_";
		push @result, $d;
		push @result, expand_dist_subdirs($d);
	    }
	    last;
        }
    }
    close M;
    return @result;
}

sub tohtml($) {
  my $html = $_[0];
  $html =~ s#&#&amp;#g;
  $html =~ s#>#&gt;#g;
  $html =~ s#<#&lt;#g;
  $html =~ s#"#&quot;#g;
  $html =~ s#\n\n#\n<P>\n#g;
  return $html;
}

sub output_html() {
    # Open output
    open DESTFILE, ">$dest-$$.tmp" or die "$dest-$$.tmp: $!";

    # Print header
    print DESTFILE <<EOF;
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<HTML>
<HEAD>
<TITLE>Xapian: Code structure</TITLE>
</HEAD>
<BODY BGCOLOR="white">
This documentation was automatically generated, and corresponds to version
$version of Xapian.
<HR>
EOF

    # Print directory index
    print DESTFILE "<H1>Directory structure index</H1>\n";
    my $level=0;
    my $dir;
    foreach $dir (sort keys(%descriptions)) {
	my $newlev = ($dir =~ tr#/##) + 1; # Count the number of /'s in $dir
	if($level == $newlev) {
	    print DESTFILE "</LI>";
	}
	while($level < $newlev) {
	    print DESTFILE "\n" . " " x ($level * 2) . "<UL>";
	    $level++;
	}
	while($level > $newlev) {
	    print DESTFILE "</LI>\n" . " " x ($level * 2 - 2) . "</UL>";
	    $level--;
	}
	print DESTFILE "\n" . " " x ($level * 2 - 1) .
	"<LI><A HREF=\"#$dir\">$dir</A>";
    }
    my $newlev=0;
    while($level > $newlev) {
	print DESTFILE "</LI>\n" . " " x ($level * 2 - 2) . "</UL>";
	$level--;
    }
    print DESTFILE "\n<HR>\n\n";

    # Print directory details
    print DESTFILE "<H1>Directory structure</H1>\n";
    foreach $dir (sort keys(%descriptions)) {
	print DESTFILE "<A NAME=\"$dir\"></A>";
	if($webroot) {
	    if($dir eq "ROOT") {
		print DESTFILE "<A HREF=\"$webroot\">";
	    } else {
		print DESTFILE "<A HREF=\"$webroot/$dir\">";
	    }
	    print DESTFILE "<H2>$dir</H2></A>\n\n";
	} else {
	    print DESTFILE "<H2>$dir</H2>\n\n";
	}
	print DESTFILE &tohtml($descriptions{$dir});
	print DESTFILE "\n\n\n";
    }

    # Print footer
    my $date = `date "+%d %B %Y"`;
    chomp $date;

    print DESTFILE <<EOF;
<HR>
Generated on $date.
<P>
Command line used to generate this documentation:<BR>
<CODE>$0 @ARGV</CODE>
</BODY>
</HTML>
EOF

    # Close temp file and rename into place
    close DESTFILE or die "$dest-$$.tmp: $!";
    unlink $dest;
    rename "$dest-$$.tmp", $dest or die "$dest: $!";
}
