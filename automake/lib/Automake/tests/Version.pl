# Copyright (C) 2002, 2003  Free Software Foundation, Inc.
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

use Automake::Version;

my $failed = 0;

sub test_version_compare
{
  my ($left, $right, $result) = @_;
  my @leftver = Automake::Version::split ($left);
  my @rightver = Automake::Version::split ($right);
  if ($#leftver == -1)
  {
    print "can't grok \"$left\"\n";
    $failed = 1;
    return;
  }
  if ($#rightver == -1)
  {
    print "can't grok \"$right\"\n";
    $failed = 1;
    return;
  }
  my $res = Automake::Version::compare (@leftver, @rightver);
  if ($res != $result)
  {
    print "compare (\"$left\", \"$right\") = $res! (not $result?)\n";
    $failed = 1;
  }
}

my @tests = (
# basics
  ['1.0', '2.0', -1],
  ['2.0', '1.0', 1],
  ['1.2', '1.2', 0],
  ['1.1', '1.2', -1],
  ['1.2', '1.1', 1],
# alphas
  ['1.4', '1.4g', -1],
  ['1.4g', '1.5', -1],
  ['1.4g', '1.4', 1],
  ['1.5', '1.4g', 1],
  ['1.4a', '1.4g', -1],
  ['1.5a', '1.3g', 1],
  ['1.6a', '1.6a', 0],
# micros
  ['1.5.1', '1.5', 1],
  ['1.5.0', '1.5', 0],
  ['1.5.4', '1.6.1', -1],
# micros and alphas
  ['1.5a', '1.5.1', 1],
  ['1.5a', '1.5.1a', 1],
  ['1.5a', '1.5.1f', 1],
  ['1.5', '1.5.1a', -1],
  ['1.5.1a', '1.5.1f', -1],
# special exceptions
  ['1.6-p5a', '1.6.5a', 0],
  ['1.6', '1.6-p5a', -1],
  ['1.6-p4b', '1.6-p5a', -1],
  ['1.6-p4b', '1.6-foo', 1],
  ['1.6-p4b', '1.6a-foo', -1]
);

test_version_compare (@{$_}) foreach @tests;

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
