use ExtUtils::MakeMaker;
use strict;
use Config;

# Suppress warnings about parameters we allow the user to specify.
$ExtUtils::MakeMaker::Recognized_Att_Keys{CPPFLAGS} = 1;
$ExtUtils::MakeMaker::Recognized_Att_Keys{CXX} = 1;
$ExtUtils::MakeMaker::Recognized_Att_Keys{CXXFLAGS} = 1;
$ExtUtils::MakeMaker::Recognized_Att_Keys{XAPIAN_CONFIG} = 1;

my $builddir;

my $srcdir = $0;
if ($srcdir =~ s!/([^/]*)$!!) {
    # Set $0 to be just the leafname.  If we don't, WriteMakefile() reruns this
    # script for reasons unknown, leading to a seemingly infinite loop
    # consuming increasing amounts of memory.  With setting $0, it still reruns
    # this script, but only once.
    $0 = $1;
    chomp($builddir = `pwd`);
    chdir $srcdir;
}

my $xapian_config;
my $CC;
my %var = ();

for (@ARGV) {
    if (/^XAPIAN_CONFIG=(.*)/) {
	$xapian_config = $1;
    } elsif (/^CXX=(.*)/) {
	$CC = $1;
    } elsif (/^(C(?:XX|PP)FLAGS)=(.*)/) {
	$var{$1} = $2;
    }
}

if (!defined $xapian_config && exists $ENV{XAPIAN_CONFIG}) {
    $xapian_config = $ENV{XAPIAN_CONFIG};
    push @ARGV, "XAPIAN_CONFIG=$xapian_config";
}
$xapian_config ||= 'xapian-config';

if (!defined $CC && exists $ENV{CXX}) {
    $CC = $ENV{CXX};
    push @ARGV, "CXX=$CC";
}
$CC ||= 'g++';

my $LD = '$(CC)';
if ($^O eq 'cygwin' and $CC eq 'g++') {
    # Cygwin packages of Perl < 5.9.5 used "ld2" for $Config{ld} and
    # $Config{lddlflags} didn't contain -shared so we need to specify
    # this explicitly.  Perl >= 5.9.5 package do away with "ld2", but
    # it should be harmless to specify "-shared" there.
    $LD = 'g++ -shared';
}

my $xver = `$xapian_config --version`;
if ($xver eq '') {
    print STDERR <<END;
$xapian_config not found.

You need Xapian installed before you can build SymbolTest.  If you have
installed Xapian from a package, you will also need to install the correspoding
-dev or -devel package.  If Xapian is installed but xapian-config isn't on your
PATH you can tell Makefile.PL this by running it like so:

    perl Makefile.PL XAPIAN_CONFIG=/path/to/xapian-config
END
    # Perversely, the CPAN automatic testing script expects exit status 0 to
    # indicate "can't build because of missing dependencies" (which it
    # distinguishes from "all OK" by seeing if Makefile is generated).  So we
    # exit with status 0 in this case to avoid being spammed with useless
    # "bug" reports from testers without Xapian installed.
    exit(0);
}
chomp($xver);
$xver =~ s/.*\s//; # "xapian 1.2.0" -> "1.2.0"

my $inc = `$xapian_config --cxxflags`;
chomp($inc);

my @writemakefile_args = ();
my $libsvar = 'LIBS';
my $libs = `$xapian_config --libs 2> /dev/null`;
chomp($libs);
my ($xapian_config_dir) = $xapian_config =~ /^(.*?)[^\/]*$/;
if ($? || ($xapian_config_dir ne '' && -f "${xapian_config_dir}Makefile")) {
    # Assume we're being asked to build against an uninstalled xapian-core.
    my $libtool = "${xapian_config_dir}libtool";
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
    $LD = "$libtool --tag=CXX --mode=link $CC -avoid-version -module";
    $LD .= " -rpath \$(PERL_ARCHLIB)/auto/\$(FULLEXT)";
    $LD .= " -shrext .".$Config{'dlext'};
    $CC = "$libtool --tag=CXX --mode=compile $CC";
    push @writemakefile_args, (
	'OBJ_EXT'	=> '.lo',
	'DLEXT'		=> 'la',
	'FULLPERL'	=> '$(PERL) "-I$(INST_ARCHAUTODIR)/.libs"',
    );
}

# Filter out some gcc options which g++ doesn't support.
my $CCFLAGS = $Config{'ccflags'};
# Perl is built with -Wdeclaration-after-statement on RHEL5 - this isn't
# meaningful for C++ - it only emits a warning but it's easy to fix.
$CCFLAGS =~ s/(?:^|\s+)-Wdeclaration-after-statement(?:\s+|$)/ /;
# The generated code causes "variable may be used uninitialized" warnings
# if Perl was built with -Wall.
$CCFLAGS =~ s/(^|\s+)-Wall(\s+|$)/$1-Wall -Wno-uninitialized$2/;

if (exists $var{CPPFLAGS}) {
    $CCFLAGS .= ' ' . $var{CPPFLAGS};
} elsif (exists $ENV{CPPFLAGS}) {
    $CCFLAGS .= ' ' . $ENV{CPPFLAGS};
}
if (exists $var{CXXFLAGS}) {
    $CCFLAGS .= ' ' . $var{CXXFLAGS};
} elsif (exists $ENV{CXXFLAGS}) {
    $CCFLAGS .= ' ' . $ENV{CXXFLAGS};
}

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
push @writemakefile_args, (
    'NAME'		=> 'SymbolTest',
    'VERSION_FROM'	=> 'SymbolTest.pm', # finds $VERSION
    'PREREQ_PM'		=> {}, # e.g., Module::Name => 1.1
    $libsvar		=> $libs, # e.g., '-lm'
    'DEFINE'		=> '', # e.g., '-DHAVE_SOMETHING'
    'CC'		=> $CC,
    'CCFLAGS'		=> $CCFLAGS,
    'LD'		=> $LD,
    'INC'		=> $inc, # e.g., '-I/usr/include/other'
    'OBJECT'		=> '$(BASEEXT)$(OBJ_EXT)',
    'XSOPT'		=> '-C++',
);
WriteMakefile(@writemakefile_args);
