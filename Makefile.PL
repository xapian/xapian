use ExtUtils::MakeMaker;

$CC = 'g++';

# locate xapian header and shared object files
# using `locate` is decidedly ugly, but the best way I can think to do
# so for the time being. 
my $lib = locate( 'libxapian.so' );
my $inc = locate( 'xapian.h' );

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'		=> 'Search::Xapian',
    'VERSION_FROM'	=> 'Xapian.pm', # finds $VERSION
    'PREREQ_PM'		=> {}, # e.g., Module::Name => 1.1
    ($] >= 5.005 ?    ## Add these new keywords supported since 5.005
      (ABSTRACT_FROM => 'Xapian.pm', # retrieve abstract from module
       AUTHOR     => 'Alex Bowley <kilinrax@cpan.org>') : ()),
    'LIBS'		=> "-L$lib -lxapian", # e.g., '-lm'
    'DEFINE'		=> '', # e.g., '-DHAVE_SOMETHING'
    'CC'                => $CC,
    'LD'                => '$(CC)',
	# Insert -I. if you add *.h files later:
    'INC'		=> "-I$inc", # e.g., '-I/usr/include/other'
	# Un-comment this if you add C files to link with later:
    # 'OBJECT'		=> '$(O_FILES)', # link all the C files too
    'XSOPT'             => '-C++',
    'TYPEMAPS'          => ['perlobject.map','typemap'],
);

sub locate($) {
  my $file = shift;
  print "Searching for $file:";
  my @matches = `locate "*/$file"`;
  map { chomp } @matches;
  @matches = sort { -M $a <=> -M $b } grep { -e $_ } @matches;
  my $result = $matches[0];
  $result =~ s|/\Q$file\E$||;
  print " found in '$result'\n";
  return $result;
}

sub MY::postamble {
  return "\$(XS_FILES): \$(wildcard XS/*.xs)\n\ttouch \$(XS_FILES)";
}
