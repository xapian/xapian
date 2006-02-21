use ExtUtils::MakeMaker;

my $CC = $ENV{"CXX"} || 'g++';

my $xapian_config = $ENV{XAPIAN_CONFIG} || 'xapian-config';
my $xver = `$xapian_config --version`;
if ($xver eq '') {
    die("$xapian_config not found - set XAPIAN_CONFIG if Xapian isn't installed on your PATH.\n");
}
my $libs = `$xapian_config --libs`;
my $inc  = `$xapian_config --cxxflags`;
chomp($xver);
chomp($libs);
chomp($inc);
$xver =~ s/.*\s//; # xapian 0.9.3 -> 0.9.3

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'		=> 'Search::Xapian',
    'VERSION_FROM'	=> 'Xapian.pm', # finds $VERSION
    'PREREQ_PM'		=> {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'Xapian.pm', # retrieve abstract from module
       AUTHOR     => 'Alex Bowley <kilinrax@cpan.org>') : ()),
    'LIBS'		=> $libs, # e.g., '-lm'
    'DEFINE'		=> '', # e.g., '-DHAVE_SOMETHING'
    'CC'                => $CC,
    'LD'                => '$(CC)',
	# Insert -I. if you add *.h files later:
    'INC'		=> $inc, # e.g., '-I/usr/include/other'
	# Un-comment this if you add C files to link with later:
    # 'OBJECT'		=> '$(O_FILES)', # link all the C files too
    'XSOPT'             => '-C++',
    'TYPEMAPS'          => ['perlobject.map','typemap'],
);

my $VERSION = "unknown";
open F, "Makefile" or die $!;
while (<F>) {
    if (/^VERSION\s*=\s*(\S*)/) {
	$VERSION = $1;
	last;
    }
}
close F;

if ($VERSION !~ /^\Q$xver\E(?:\..*)?$/) {
    warn "Warning: Xapian version $xver may be incompatible with Search::Xapian $VERSION\n";
}

my @bad;
for my $file (qw(Changes META.yml README)) {
    open F, $file or next;
    my $ok;
    while (<F>) {
	if (/\b\Q$VERSION\E\b/) {
	    $ok = 1;
	    last;
	}
    }
    close F;
    if (!$ok) {
	push @bad, $file;
    }
}
if (scalar @bad) {
    unlink "Makefile";
    die(join(",",@bad).": No mention current version: $VERSION\n");
}


sub MY::postamble {
  return "\$(XS_FILES): ".join(" ", <XS/*.xs>)."\n\ttouch \$(XS_FILES)";
}
