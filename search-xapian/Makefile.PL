use ExtUtils::MakeMaker;

my $CC = 'g++';

my $xapian_config = $ENV{XAPIAN_CONFIG} || 'xapian-config';
my $libs = `$xapian_config --libs`;
my $inc  = `$xapian_config --cxxflags`;

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


sub MY::postamble {
  return "\$(XS_FILES): \$(wildcard XS/*.xs)\n\ttouch \$(XS_FILES)";
}
