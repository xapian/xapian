# Copyright (C) 2001, 2002, 2003  Free Software Foundation, Inc.
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
# along with autoconf; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301, USA.

use Automake::Condition qw/TRUE FALSE/;

sub test_basics ()
{
  my @tests = (# [[Conditions], is_true?, is_false?, string, subst-string]
	       [[], 1, 0, 'TRUE', ''],
	       [['TRUE'], 1, 0, 'TRUE', ''],
	       [['FALSE'], 0, 1, 'FALSE', '#'],
	       [['A_TRUE'], 0, 0, 'A_TRUE', '@A_TRUE@'],
	       [['A_TRUE', 'B_FALSE'],
		0, 0, 'A_TRUE B_FALSE', '@A_TRUE@@B_FALSE@'],
	       [['B_TRUE', 'FALSE'], 0, 1, 'FALSE', '#'],
	       [['B_TRUE', 'B_FALSE'], 0, 1, 'FALSE', '#']);

  for (@tests)
    {
      my $a = new Automake::Condition @{$_->[0]};
      return 1 if $_->[1] != $a->true;
      return 1 if $_->[1] != ($a == TRUE);
      return 1 if $_->[2] != $a->false;
      return 1 if $_->[2] != ($a == FALSE);
      return 1 if $_->[3] ne $a->string;
      return 1 if $_->[4] ne $a->subst_string;
    }
  return 0;
}

sub test_true_when ()
{
  my $failed = 0;

  my @tests = (# [When,
	       #  [Implied-Conditions],
	       #  [Not-Implied-Conditions]]
	       [['TRUE'],
		[['TRUE']],
		[['A_TRUE'], ['A_TRUE', 'B_FALSE'], ['FALSE']]],
	       [['A_TRUE'],
		[['TRUE'], ['A_TRUE']],
		[['A_TRUE', 'B_FALSE'], ['FALSE']]],
	       [['A_TRUE', 'B_FALSE'],
		[['TRUE'], ['A_TRUE'], ['B_FALSE'], ['A_TRUE', 'B_FALSE']],
		[['FALSE'], ['C_FALSE'], ['C_FALSE', 'A_TRUE']]]);

  for my $t (@tests)
    {
      my $a = new Automake::Condition @{$t->[0]};
      for my $u (@{$t->[1]})
	{
	  my $b = new Automake::Condition @$u;
	  if (! $b->true_when ($a))
	    {
	      print "`" . $b->string .
		"' not implied by `" . $a->string . "'?\n";
	      $failed = 1;
	    }
	}
      for my $u (@{$t->[2]})
	{
	  my $b = new Automake::Condition @$u;
	  if ($b->true_when ($a))
	    {
	      print "`" . $b->string .
		"' implied by `" . $a->string . "'?\n";
	      $failed = 1;
	    }

	  return 1 if $b->true_when ($a);
	}
    }
  return $failed;
}

sub test_reduce_and ()
{
  my @tests = (# If no conditions are given, TRUE should be returned
	       [[], ["TRUE"]],
	       # An empty condition is TRUE
	       [[""], ["TRUE"]],
	       # A single condition should be passed through unchanged
	       [["FOO"], ["FOO"]],
	       [["FALSE"], ["FALSE"]],
	       [["TRUE"], ["TRUE"]],
	       # TRUE and false should be discarded and overwhelm
	       # the result, respectively
	       [["FOO", "TRUE"], ["FOO"]],
	       [["FOO", "FALSE"], ["FALSE"]],
	       # Repetitions should be removed
	       [["FOO", "FOO"], ["FOO"]],
	       [["TRUE", "FOO", "FOO"], ["FOO"]],
	       [["FOO", "TRUE", "FOO"], ["FOO"]],
	       [["FOO", "FOO", "TRUE"], ["FOO"]],
	       # Two different conditions should be preserved,
	       # but TRUEs should be removed
	       [["FOO", "BAR"], ["BAR,FOO"]],
	       [["TRUE", "FOO", "BAR"], ["BAR,FOO"]],
	       [["FOO", "TRUE", "BAR"], ["BAR,FOO"]],
	       [["FOO", "BAR", "TRUE"], ["BAR,FOO"]],
	       # A condition implied by another condition should be removed.
	       [["FOO BAR", "BAR"], ["FOO BAR"]],
	       [["BAR", "FOO BAR"], ["FOO BAR"]],
	       [["TRUE", "FOO BAR", "BAR"], ["FOO BAR"]],
	       [["FOO BAR", "TRUE", "BAR"], ["FOO BAR"]],
	       [["FOO BAR", "BAR", "TRUE"], ["FOO BAR"]],

	       [["BAR FOO", "BAR"], ["BAR FOO"]],
	       [["BAR", "BAR FOO"], ["BAR FOO"]],
	       [["TRUE", "BAR FOO", "BAR"], ["BAR FOO"]],
	       [["BAR FOO", "TRUE", "BAR"], ["BAR FOO"]],
	       [["BAR FOO", "BAR", "TRUE"], ["BAR FOO"]],

	       # Check that reduction happens even when there are
	       # two conditions to remove.
	       [["FOO", "FOO BAR", "BAR"], ["FOO BAR"]],
	       [["FOO", "FOO BAR", "BAZ", "FOO BAZ"], ["FOO BAR", "FOO BAZ"]],
	       [["FOO", "FOO BAR", "BAZ", "FOO BAZ", "FOO BAZ BAR"],
		["FOO BAZ BAR"]],

	       # Duplicated conditionals should be removed.
	       [["FOO", "BAR", "BAR"], ["BAR,FOO"]],

	       # Equivalent conditions in different forms should be
	       # reduced: which one is left is unfortunately order
	       # dependent.
	       [["BAR FOO", "FOO BAR"], ["FOO BAR"]],
	       [["FOO BAR", "BAR FOO"], ["BAR FOO"]]);

  my $failed = 0;
  foreach (@tests)
    {
      my ($inref, $outref) = @$_;
      my @inconds = map { new Automake::Condition $_ } @$inref;
      my @outconds = map { (new Automake::Condition $_)->string } @$outref;
      my @res =
	map { $_->string } (Automake::Condition::reduce_and (@inconds));
      my $result = join (",", sort @res);
      my $exresult = join (",", @outconds);

      if ($result ne $exresult)
	{
	  print '"' . join(",", @$inref) . '" => "' .
	    $result . '" expected "' .
	      $exresult . '"' . "\n";
	  $failed = 1;
	}
    }
  return $failed;
}

sub test_reduce_or ()
{
  my @tests = (# If no conditions are given, FALSE should be returned
	       [[], ["FALSE"]],
	       # An empty condition is TRUE
	       [[""], ["TRUE"]],
	       # A single condition should be passed through unchanged
	       [["FOO"], ["FOO"]],
	       [["FALSE"], ["FALSE"]],
	       [["TRUE"], ["TRUE"]],
	       # FALSE and TRUE should be discarded and overwhelm
	       # the result, respectively
	       [["FOO", "TRUE"], ["TRUE"]],
	       [["FOO", "FALSE"], ["FOO"]],
	       # Repetitions should be removed
	       [["FOO", "FOO"], ["FOO"]],
	       [["FALSE", "FOO", "FOO"], ["FOO"]],
	       [["FOO", "FALSE", "FOO"], ["FOO"]],
	       [["FOO", "FOO", "FALSE"], ["FOO"]],
	       # Two different conditions should be preserved,
	       # but FALSEs should be removed
	       [["FOO", "BAR"], ["BAR,FOO"]],
	       [["FALSE", "FOO", "BAR"], ["BAR,FOO"]],
	       [["FOO", "FALSE", "BAR"], ["BAR,FOO"]],
	       [["FOO", "BAR", "FALSE"], ["BAR,FOO"]],
	       # A condition implying another condition should be removed.
	       [["FOO BAR", "BAR"], ["BAR"]],
	       [["BAR", "FOO BAR"], ["BAR"]],
	       [["FALSE", "FOO BAR", "BAR"], ["BAR"]],
	       [["FOO BAR", "FALSE", "BAR"], ["BAR"]],
	       [["FOO BAR", "BAR", "FALSE"], ["BAR"]],

	       [["BAR FOO", "BAR"], ["BAR"]],
	       [["BAR", "BAR FOO"], ["BAR"]],
	       [["FALSE", "BAR FOO", "BAR"], ["BAR"]],
	       [["BAR FOO", "FALSE", "BAR"], ["BAR"]],
	       [["BAR FOO", "BAR", "FALSE"], ["BAR"]],

	       # Check that reduction happens even when there are
	       # two conditions to remove.
	       [["FOO", "FOO BAR", "BAR"], ["BAR,FOO"]],
	       [["FOO", "FOO BAR", "BAZ", "FOO BAZ"], ["BAZ,FOO"]],
	       [["FOO", "FOO BAR", "BAZ", "FOO BAZ", "FOO BAZ BAR"],
		["BAZ,FOO"]],

	       # Duplicated conditionals should be removed.
	       [["FOO", "BAR", "BAR"], ["BAR,FOO"]],

	       # Equivalent conditions in different forms should be
	       # reduced: which one is left is unfortunately order
	       # dependent.
	       [["BAR FOO", "FOO BAR"], ["FOO BAR"]],
	       [["FOO BAR", "BAR FOO"], ["BAR FOO"]]);

  my $failed = 0;
  foreach (@tests)
    {
      my ($inref, $outref) = @$_;
      my @inconds = map { new Automake::Condition $_ } @$inref;
      my @outconds = map { (new Automake::Condition $_)->string } @$outref;
      my @res =
	map { $_->string } (Automake::Condition::reduce_or (@inconds));
      my $result = join (",", sort @res);
      my $exresult = join (",", @outconds);

      if ($result ne $exresult)
	{
	  print '"' . join(",", @$inref) . '" => "' .
	    $result . '" expected "' .
	      $exresult . '"' . "\n";
	  $failed = 1;
	}
    }
  return $failed;
}

exit (test_basics || test_true_when || test_reduce_and || test_reduce_or);

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
