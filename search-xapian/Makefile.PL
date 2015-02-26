use ExtUtils::MakeMaker;
use strict;
use Config;


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

open ARGS, '>', 'makefile-pl-args' or die $!;
for (@ARGV) {
    print ARGS $_, "\n";
    if (/^XAPIAN_CONFIG=(.*)/) {
	$xapian_config = $1;
    } elsif (/^CXX=(.*)/) {
	$CC = $1;
    } elsif (/^(C(?:XX|PP)FLAGS)=(.*)/) {
	$var{$1} = $2;
    }
}
close ARGS or die $!;

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

You need Xapian installed before you can build Search::Xapian.  If you have
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
    open LDWRAP, '>', 'ld-wrapper' or die $!;
    print LDWRAP <<END;
#!/bin/sh
set -e
SO=\$1
INST_ARCHAUTODIR=\$2
shift
shift
"\$\@"
ln -sf ".libs/Xapian.\$SO" "\$INST_ARCHAUTODIR/Xapian.\$SO"
END
    chmod 0755, *LDWRAP or die $!;
    close LDWRAP or die $!;
    $LD = "./ld-wrapper \"\$(SO)\" \"\$(INST_ARCHAUTODIR)\"";
    $LD .= " $libtool --tag=CXX --mode=link $CC -avoid-version -module";
    $LD .= " -rpath \$(PERL_ARCHLIB)/auto/\$(FULLEXT)";
    $LD .= " -shrext .".$Config{'dlext'};
    $CC = "$libtool --tag=CXX --mode=compile $CC";
    push @writemakefile_args, (
	'OBJ_EXT'	=> '.lo',
	'DLEXT'		=> 'la'
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
    'NAME'		=> 'Search::Xapian',
    'VERSION_FROM'	=> 'Xapian.pm', # finds $VERSION
    'PREREQ_PM'		=> {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'Xapian.pm', # retrieve abstract from module
       AUTHOR     => 'Alex Bowley <xapian-discuss@lists.xapian.org>') : ()),
#       AUTHOR     => 'Alex Bowley <kilinrax@cpan.org>') : ()),
    $libsvar		=> $libs, # e.g., '-lm'
    'BUILD_REQUIRES'	=> { 'Devel::Leak' => 0 },
    'DEFINE'		=> '', # e.g., '-DHAVE_SOMETHING'
    'CC'		=> $CC,
    'CCFLAGS'		=> $CCFLAGS,
    'LD'		=> $LD,
    'INC'		=> $inc, # e.g., '-I/usr/include/other'
    'OBJECT'		=> '$(BASEEXT)$(OBJ_EXT) handle_exception$(OBJ_EXT)',
    'XSOPT'		=> '-C++',
    # 'typemap' is implicitly added to this list.
    'TYPEMAPS'		=> ['perlobject.map', 'typemap-errorclasses'],
    'clean'		=> { 'FILES' => 'ld-wrapper' },
    # Add "make check" as alias for "make test".
    # Make sure that we rebuild the Makefile if the version number changes.
    'depend'		=> { 'check' => 'test', 'Makefile' => 'Xapian.pm' },
);

# Throw away STDOUT from WriteMakefile() to suppress warnings about parameters
# we allow the user to specify: CPPFLAGS CXX CXXFLAGS XAPIAN_CONFIG
*TEMP = *STDOUT;
close STDOUT;
WriteMakefile(@writemakefile_args);
*STDOUT = *TEMP;

my $VERSION = "unknown";
open F, "Makefile" or die $!;
while (<F>) {
    if (/^VERSION\s*=\s*(\S*)/) {
	$VERSION = $1;
	last;
    }
}
close F;

my ($BASEVERSION) = $VERSION =~ /^([0-9]+\.[0-9]+\.[0-9]+)/;
if ($xver !~ /^\Q$BASEVERSION\E(?:_svn[0-9]+)?$/) {
    # We definitely need Xapian 1.2.x.
    my $no_chance = ($xver !~ /^1\.2\./);
    my $msg;
    if ($no_chance) {
	$msg = "Xapian version $xver is incompatible with Search::Xapian $VERSION\n";
    } else {
	$msg = "Xapian version $xver may be incompatible with Search::Xapian $VERSION\n";
    }

    if ($ENV{AUTOMATED_TESTING}) {
	# Don't let automated testers continue, as we don't want bogus failure
	# reports due to builds with incompatible versions.
	print $msg;
	# Remove Makefile since "exit status 0 without generating Makefile"
	# is taken to indicate "can't build because of missing dependencies"
	# by CPAN test building scripts.
	unlink "Makefile";
	exit 0;
    }
    if ($no_chance) {
	unlink "Makefile";
	die $msg;
    }
    warn "Warning: $msg";
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
    die(join(",",@bad).": No mention of current version: $VERSION\n");
}

# If we're doing a fake VPATH build, add a stub Makefile which forwards all
# invocations (.DEFAULT is a GNU-make-ism).
if (defined $builddir) {
    open M, '>', "$builddir/Makefile~" or die $!;
    print M <<"EOF";
all .DEFAULT:
\t\$(MAKE) -C "$srcdir" \$\@

.PHONY: all Search-Xapian-$VERSION.tar.gz

dist tardist Search-Xapian-$VERSION.tar.gz:
\t\$(MAKE) -C "$srcdir" \$\@
\trm -f Search-Xapian-$VERSION.tar.gz
\tcp "$srcdir"/Search-Xapian-$VERSION.tar.gz .
EOF

    close M or die $!;
    rename "$builddir/Makefile~", "$builddir/Makefile" or die $!;
}

sub MY::postamble {
  return "\$(XS_FILES): ".join(" ", <XS/*.xs>)."\n\ttouch \$(XS_FILES)";
}
