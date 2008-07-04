# Copyright (C) 2003  Free Software Foundation, Inc.
#
# This file is part of GNU Automake.
#
# GNU Automake is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# GNU Automake is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Automake; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301, USA.

use Automake::Wrap 'wrap';

my $failed = 0;

sub test_wrap
{
  my ($in, $exp_out) = @_;

  my $out = &wrap (@$in);
  if ($out ne $exp_out)
    {
      print STDERR "For: @$in\nGot:\n$out\nInstead of:\n$exp_out\n---\n";
      ++$failed;
    }
}

my @tests = (
  [["HEAD:", "NEXT:", "CONT", 13, "v" ,"a", "l", "ue", "s", "values"],
"HEAD:v aCONT
NEXT:l ueCONT
NEXT:sCONT
NEXT:values
"],
  [["rule: ", "\t", " \\", 20, "dep1" ,"dep2", "dep3", "dep4", "dep5",
    "dep06", "dep07", "dep08"],
"rule: dep1 dep2 \\
\tdep3 dep4 \\
\tdep5 dep06 \\
\tdep07 \\
\tdep08
"],
  [["big header:", "big continuation:", " END", 5, "diag1", "diag2", "diag3"],
"big header:diag1 END
big continuation:diag2 END
big continuation:diag3
"],
  [["big header:", "cont: ", " END", 16, "word1", "word2"],
"big header: END
cont: word1 END
cont: word2
"]);


test_wrap (@{$_}) foreach @tests;

exit $failed;

### Setup "GNU" style for perl-mode and cperl-mode.
## Local Variables:
## perl-indent-level: 2
## perl-continued-statement-offset: 2
## perl-continued-brace-offset: 0
## perl-brace-offset: 0
## perl-brace-imaginary-offset: 0
## perl-label-offset: -2
## cperl-indent-level: 2
## cperl-brace-offset: 0
## cperl-continued-brace-offset: 0
## cperl-label-offset: -2
## cperl-extra-newline-before-brace: t
## cperl-merge-trailing-else: nil
## cperl-continued-statement-offset: 2
## End:
