package SymbolTest;
use strict;
use warnings;
use vars qw( $VERSION @ISA );

$VERSION = '1.2.3.0';

require DynaLoader;
@ISA = qw( DynaLoader );

# We need to use the RTLD_GLOBAL flag to dlopen() so that other C++
# modules that link against libxapian.so get the *same* value for all the
# weak symbols (eg, the exception classes)
#### sub dl_load_flags { 0x01 }

bootstrap SymbolTest $VERSION;

1;
