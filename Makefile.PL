use ExtUtils::MakeMaker;
use strict;

my $CC = $ENV{"CXX"} || 'g++';
my $LD = '$(CC)';

my $xapian_config = $ENV{XAPIAN_CONFIG} || 'xapian-config';

my $xver = `$xapian_config --version`;
if ($xver eq '') {
    die "$xapian_config not found - set XAPIAN_CONFIG if Xapian isn't installed on your PATH.\n";
}
chomp($xver);
$xver =~ s/.*\s//; # "xapian 0.9.3" -> "0.9.3"

my $inc  = `$xapian_config --cxxflags`;
chomp($inc);

my $libsvar = 'LIBS';
my $libs = `$xapian_config --libs 2> /dev/null`;
chomp($libs);
my ($xapian_config_dir) = $xapian_config =~ /^(.*?)[^\/]*$/;
if ($? || -f "$xapian_config_dir/Makefile") {
    # Assume we're being asked to build against an uninstalled xapian-core.
    my $libtool = "$xapian_config_dir/libtool";
    unless (-x $libtool) {
	die "You've asked me to link against what appears to be an uninstalled xapian-core tree, but I can't find libtool in that tree\n";
    }

    # We can't pass a .la file in LIBS since MakeMaker "knows better" and
    # ignores it.  Passing it in LDLOADLIBS works, but generates a warning.
    # We can avoid the warning by setting LDLOADLIBS using 'macro'.
    $libsvar = 'macro';
    $libs = `$xapian_config --ltlibs`;
    chomp($libs);
    $libs = {'LDLOADLIBS' => $libs};
    $LD = "$libtool --tag=CXX --mode=link $CC -avoid-version -module -no-install";
    $CC = "$libtool --mode=compile $CC";
}

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'		=> 'Search::Xapian',
    'VERSION_FROM'	=> 'Xapian.pm', # finds $VERSION
    'PREREQ_PM'		=> {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'Xapian.pm', # retrieve abstract from module
       AUTHOR     => 'Alex Bowley <xapian-discuss@lists.xapian.org>') : ()),
#       AUTHOR     => 'Alex Bowley <kilinrax@cpan.org>') : ()),
    $libsvar		=> $libs, # e.g., '-lm'
    'DEFINE'		=> '', # e.g., '-DHAVE_SOMETHING'
    'CC'                => $CC,
    'LD'                => $LD,
	# Insert -I. if you add *.h files later:
    'INC'		=> $inc, # e.g., '-I/usr/include/other'
	# Un-comment this if you add C files to link with later:
    # 'OBJECT'		=> '$(O_FILES)', # link all the C files too
    'XSOPT'             => '-C++',
    'TYPEMAPS'          => ['perlobject.map','typemap'],
    # Add "make check" as alias for "make test".
    'depend'		=> { 'check' => 'test' },
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
for my $file (qw(Changes README)) {
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
