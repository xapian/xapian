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
use Automake::DisjConditions;

sub test_basics ()
{
  my $cond = new Automake::Condition "COND1_TRUE", "COND2_FALSE";
  my $other = new Automake::Condition "COND3_FALSE";
  my $set1 = new Automake::DisjConditions $cond, $other;
  my $set2 = new Automake::DisjConditions $other, $cond;
  return 1 unless $set1 == $set2;
  return 1 if $set1->false;
  return 1 if $set1->true;
  return 1 unless (new Automake::DisjConditions)->false;
  return 1 if (new Automake::DisjConditions)->true;
}

sub build_set (@)
{
  my @conds = @_;
  my @set = ();
  for my $cond (@conds)
    {
      push @set, new Automake::Condition @$cond;
    }
  return new Automake::DisjConditions @set;
}

sub test_invert ()
{
  my @tests = ([[["FALSE"]],
	        [["TRUE"]]],

	       [[["TRUE"]],
	        [["FALSE"]]],

	       [[["COND1_TRUE", "COND2_TRUE"],
		 ["COND3_FALSE", "COND2_TRUE"]],
		[["COND2_FALSE"],
		 ["COND1_FALSE", "COND3_TRUE"]]],

	       [[["COND1_TRUE", "COND2_TRUE"],
		 ["TRUE"]],
		[["FALSE"]]],

	       [[["COND1_TRUE", "COND2_TRUE"],
		 ["FALSE"]],
		[["COND1_FALSE"],
		 ["COND2_FALSE"]]],

	       [[["COND1_TRUE"],
		 ["COND2_FALSE"]],
		[["COND1_FALSE", "COND2_TRUE"]]]
	       );

  for my $t (@tests)
    {
      my $set = build_set @{$t->[0]};
      my $res = build_set @{$t->[1]};
      my $inv = $set->invert;
      if ($inv != $res)
	{
	  print " (I) " . $set->string . "\n\t"
	    . $inv->string . ' != ' . $res->string . "\n";
	  return 1;
	}
    }
  return 0;
}

sub test_simplify ()
{
  my @tests = ([[["FOO_TRUE", "BAR_FALSE", "BAZ_FALSE"],
		 ["FOO_TRUE", "BAR_FALSE", "BAZ_TRUE"]],
		[["FOO_TRUE", "BAR_FALSE"]]],

	       [[["FOO_TRUE", "BAR_FALSE", "BAZ_FALSE"],
		 ["FOO_TRUE", "BAR_FALSE", "BAZ_TRUE"],
		 ["FOO_TRUE", "BAR_TRUE"]],
		[["FOO_TRUE"]]],

	       [[["FOO_TRUE", "BAR_FALSE", "BAZ_FALSE"],
		 ["FOO_TRUE", "BAR_FALSE", "BAZ_TRUE"],
		 ["FOO_TRUE", "BAR_TRUE"],
		 ["FOO_FALSE"]],
		[["TRUE"]]],

	       [[["FOO_TRUE", "BAR_FALSE", "BAZ_FALSE"],
		 ["FOO_TRUE", "BAR_FALSE", "BAZ_TRUE"],
		             ["BAR_TRUE",  "BAZ_TRUE"],
		             ["BAR_FALSE", "BAZ_TRUE"]],
		[["BAZ_TRUE"], ["FOO_TRUE", "BAR_FALSE"]]],

	       [[["FOO_TRUE", "BAR_FALSE", "BAZ_FALSE"],
		 ["FOO_TRUE", "BAR_FALSE", "BAZ_TRUE"],
		             ["BAR_TRUE",  "BAZ_TRUE"],
		             ["BAR_FALSE", "BAZ_TRUE"],
		 ["FOO_FALSE"]],
		[["FOO_FALSE"], ["BAZ_TRUE"], ["BAR_FALSE"]]],

	       [[["B_TRUE"],
		 ["A_FALSE", "B_TRUE"]],
		[["B_TRUE"]]],

	       [[["B_TRUE"],
		 ["A_FALSE", "B_FALSE", "C_TRUE"],
		 ["A_FALSE", "B_FALSE", "C_FALSE"]],
		[["A_FALSE"], ["B_TRUE"]]],

	       [[["B_TRUE"],
		 ["A_FALSE", "B_FALSE", "C_TRUE"],
		 ["A_FALSE", "B_FALSE", "C_FALSE"],
		 ["A_TRUE", "B_FALSE"]],
		[["TRUE"]]],

	       [[["A_TRUE", "B_TRUE"],
		 ["A_TRUE", "B_FALSE"],
		 ["A_TRUE", "C_FALSE", "D_FALSE"]],
		[["A_TRUE"]]],

	       [[["A_FALSE", "B_FALSE", "C_FALSE", "D_TRUE",  "E_FALSE"],
		 ["A_FALSE", "B_FALSE", "C_TRUE",  "D_TRUE",  "E_TRUE"],
		 ["A_FALSE", "B_TRUE",  "C_TRUE",  "D_FALSE", "E_TRUE"],
		 ["A_FALSE", "B_TRUE",  "C_FALSE", "D_FALSE", "E_FALSE"],
		 ["A_TRUE",  "B_TRUE",  "C_FALSE", "D_FALSE", "E_FALSE"],
		 ["A_TRUE",  "B_TRUE",  "C_TRUE",  "D_FALSE", "E_TRUE"],
		 ["A_TRUE",  "B_FALSE", "C_TRUE",  "D_TRUE",  "E_TRUE"],
		 ["A_TRUE",  "B_FALSE", "C_FALSE", "D_TRUE",  "E_FALSE"]],
		[           ["B_FALSE", "C_FALSE", "D_TRUE",  "E_FALSE"],
		            ["B_FALSE", "C_TRUE",  "D_TRUE",  "E_TRUE"],
		            ["B_TRUE",  "C_TRUE",  "D_FALSE", "E_TRUE"],
		            ["B_TRUE",  "C_FALSE", "D_FALSE", "E_FALSE"]]],

	       [[["A_FALSE", "B_FALSE", "C_FALSE", "D_TRUE",  "E_FALSE"],
		 ["A_FALSE", "B_FALSE", "C_TRUE",  "D_TRUE",  "E_TRUE"],
		 ["A_FALSE", "B_TRUE",  "C_TRUE",  "D_FALSE", "E_TRUE"],
		 ["A_FALSE", "B_TRUE",  "C_FALSE", "D_FALSE", "E_FALSE"],
		 ["A_TRUE",  "B_TRUE",  "C_FALSE", "D_FALSE", "E_FALSE"],
		 ["A_TRUE",  "B_TRUE",  "C_TRUE",  "D_FALSE", "E_TRUE"],
		 ["A_TRUE",  "B_FALSE", "C_TRUE",  "D_TRUE",  "E_TRUE"],
		 ["A_TRUE",  "B_FALSE", "C_FALSE", "D_TRUE",  "E_FALSE"],
		 ["A_FALSE", "B_FALSE", "C_FALSE", "D_FALSE", "E_FALSE"],
		 ["A_FALSE", "B_FALSE", "C_TRUE",  "D_FALSE", "E_TRUE"],
		 ["A_FALSE", "B_TRUE",  "C_TRUE",  "D_TRUE",  "E_TRUE"],
		 ["A_FALSE", "B_TRUE",  "C_FALSE", "D_TRUE",  "E_FALSE"],
		 ["A_TRUE",  "B_TRUE",  "C_FALSE", "D_TRUE",  "E_FALSE"],
		 ["A_TRUE",  "B_TRUE",  "C_TRUE",  "D_TRUE",  "E_TRUE"],
		 ["A_TRUE",  "B_FALSE", "C_TRUE",  "D_FALSE", "E_TRUE"],
		 ["A_TRUE",  "B_FALSE", "C_FALSE", "D_FALSE", "E_FALSE"]],
		[["C_FALSE", "E_FALSE"],
		 ["C_TRUE", "E_TRUE"]]],

	       [[["A_FALSE"],
		 ["A_TRUE", "B_FALSE"],
		 ["A_TRUE", "B_TRUE", "C_FALSE"],
		 ["A_TRUE", "B_TRUE", "C_TRUE", "D_FALSE"],
		 ["A_TRUE", "B_TRUE", "C_TRUE", "D_TRUE", "E_FALSE"],
		 ["A_TRUE", "B_TRUE", "C_TRUE", "D_TRUE", "E_TRUE", "F_FALSE"],
		 ["A_TRUE", "B_TRUE", "C_TRUE", "D_TRUE", "E_TRUE"]],
		[["TRUE"]]],

	       # Simplify should work with up to 31 variables.
	       [[["V01_TRUE", "V02_TRUE", "V03_TRUE", "V04_TRUE", "V05_TRUE",
		  "V06_TRUE", "V07_TRUE", "V08_TRUE", "V09_TRUE", "V10_TRUE",
		  "V11_TRUE", "V12_TRUE", "V13_TRUE", "V14_TRUE", "V15_TRUE",
		  "V16_TRUE", "V17_TRUE", "V18_TRUE", "V19_TRUE", "V20_TRUE",
		  "V21_TRUE", "V22_TRUE", "V23_TRUE", "V24_TRUE", "V25_TRUE",
		  "V26_TRUE", "V27_TRUE", "V28_TRUE", "V29_TRUE", "V30_TRUE",
		  "V31_TRUE"],
		 ["V01_TRUE", "V02_TRUE", "V03_TRUE", "V04_TRUE", "V05_TRUE",
		  "V06_TRUE", "V07_TRUE", "V08_TRUE", "V09_TRUE", "V10_TRUE",
		  "V11_TRUE", "V12_TRUE", "V13_TRUE", "V14_TRUE", "V15_TRUE",
		  "V16_TRUE", "V17_TRUE", "V18_TRUE", "V19_TRUE", "V20_TRUE",
		  "V21_TRUE", "V22_TRUE", "V23_TRUE", "V24_TRUE", "V25_TRUE",
		  "V26_TRUE", "V27_TRUE", "V28_TRUE", "V29_TRUE", "V30_TRUE",
		  "V31_FALSE"],
		 ["V01_FALSE","V02_TRUE", "V03_TRUE", "V04_TRUE", "V05_TRUE",
		  "V06_TRUE", "V07_TRUE", "V08_TRUE", "V09_TRUE", "V10_TRUE",
		  "V11_TRUE", "V12_TRUE", "V13_TRUE", "V14_TRUE", "V15_TRUE",
		  "V16_TRUE", "V17_TRUE", "V18_TRUE", "V19_TRUE", "V20_TRUE",
		  "V21_TRUE", "V22_TRUE", "V23_TRUE", "V24_TRUE", "V25_TRUE",
		  "V26_TRUE", "V27_TRUE", "V28_TRUE", "V29_TRUE", "V30_TRUE",
		  "V31_TRUE"],
		 ["V01_FALSE","V02_TRUE", "V03_TRUE", "V04_TRUE", "V05_TRUE",
		  "V06_TRUE", "V07_TRUE", "V08_TRUE", "V09_TRUE", "V10_TRUE",
		  "V11_TRUE", "V12_TRUE", "V13_TRUE", "V14_TRUE", "V15_TRUE",
		  "V16_TRUE", "V17_TRUE", "V18_TRUE", "V19_TRUE", "V20_TRUE",
		  "V21_TRUE", "V22_TRUE", "V23_TRUE", "V24_TRUE", "V25_TRUE",
		  "V26_TRUE", "V27_TRUE", "V28_TRUE", "V29_TRUE", "V30_TRUE",
		  "V31_FALSE"]],
		[[            "V02_TRUE", "V03_TRUE", "V04_TRUE", "V05_TRUE",
		  "V06_TRUE", "V07_TRUE", "V08_TRUE", "V09_TRUE", "V10_TRUE",
		  "V11_TRUE", "V12_TRUE", "V13_TRUE", "V14_TRUE", "V15_TRUE",
		  "V16_TRUE", "V17_TRUE", "V18_TRUE", "V19_TRUE", "V20_TRUE",
		  "V21_TRUE", "V22_TRUE", "V23_TRUE", "V24_TRUE", "V25_TRUE",
		  "V26_TRUE", "V27_TRUE", "V28_TRUE", "V29_TRUE", "V30_TRUE"
		  ]]]);

  for my $t (@tests)
    {
      my $set = build_set @{$t->[0]};
      my $res = build_set @{$t->[1]};

      # Make sure simplify() yields the expected result.
      my $sim = $set->simplify;
      if ($sim != $res)
	{
	  print " (S1) " . $set->string . "\n\t"
	    . $sim->string . ' != ' . $res->string . "\n";
	  return 1;
	}

      # Make sure simplify() is idempotent.
      my $sim2 = $sim->simplify;
      if ($sim2 != $sim)
	{
	  print " (S2) " . $sim->string . "\n\t"
	    . $sim2->string . ' != ' . $sim->string . "\n";
	  return 1;
	}

      # Also exercize invert() while we are at it.

      my $inv1 = $set->invert->simplify;
      my $inv2 = $sim->invert->simplify;
      if ($inv1 != $inv2)
	{
	  print " (S3) " . $set->string . ", " . $sim->string . "\n\t"
	    . $inv1->string . ' != ' . $inv2->string . "\n";
	  return 1;
	}
    }

  return 0;
}

sub test_sub_conditions ()
{
  my @tests = ([[["FOO_TRUE", "BAR_FALSE", "BAZ_FALSE"],
		 ["FOO_TRUE", "BAR_FALSE", "BAZ_TRUE"],
		 ["FOO_FALSE"]],
		["FOO_TRUE"],
		[["BAR_FALSE", "BAZ_FALSE"],
		 ["BAR_FALSE", "BAZ_TRUE"]]],

	       [[["FOO_TRUE", "BAR_FALSE", "BAZ_FALSE"],
		 ["FOO_TRUE", "BAR_FALSE", "BAZ_TRUE"],
		 ["FOO_FALSE"]],
		["FOO_TRUE", "BAR_FALSE"],
		[["BAZ_FALSE"],
		 ["BAZ_TRUE"]]],

	       [[["FOO_TRUE", "BAR_FALSE", "BAZ_FALSE"],
		 ["FOO_TRUE", "BAR_FALSE", "BAZ_TRUE"],
		 ["FOO_FALSE"]],
		["FOO_TRUE", "BAR_TRUE"],
		[["FALSE"]]],

	       [[["FOO_TRUE", "BAR_FALSE", "BAZ_FALSE"],
		 ["FOO_TRUE", "BAZ_TRUE"],
		 ["FOO_FALSE"]],
		["FOO_TRUE", "BAR_TRUE"],
		[["BAZ_TRUE"]]],

	       [[["FOO_TRUE", "BAR_FALSE"],
		 ["FOO_TRUE", "BAR_TRUE"]],
		["FOO_TRUE", "BAR_TRUE"],
		[["TRUE"]]],

	       [[["TRUE"]],
		["TRUE"],
		[["TRUE"]]],

	       [[["FALSE"]],
		["TRUE"],
		[["FALSE"]]],

	       [[["FALSE"]],
		["FALSE"],
		[["FALSE"]]]);

  for my $t (@tests)
    {
      my $t1 = build_set @{$t->[0]};
      my $t2 = new Automake::Condition @{$t->[1]};
      my $t3 = build_set @{$t->[2]};

      # Make sure sub_conditions() yields the expected result.
      my $s = $t1->sub_conditions ($t2);
      if ($s != $t3)
	{
	  print " (SC) " . $t1->string . "\n\t"
	    . $s->string . ' != ' . $t3->string . "\n";
	  return 1;
	}
    }
}

sub test_ambig ()
{
  my @tests = ([[["TRUE"]],
		["TRUE"],
		"multiply defined"],
	       [[["C1_TRUE"]],
		["C1_TRUE"],
		"multiply defined"],
	       [[["TRUE"]],
                ["C1_FALSE"],
		"which includes"],
	       [[["C1_TRUE"]],
		["C1_TRUE", "C2_TRUE"],
		"which includes"],
	       [[["C1_TRUE", "C2_TRUE"]],
		["C2_TRUE"],
		"which is included in"],
	       [[["C1_TRUE"]],
		["C2_TRUE"],
		''],
	       [[["C1_TRUE"],
		 ["C2_FALSE"]],
		["C1_FALSE", "C2_TRUE"],
		'']);

  for my $t (@tests)
    {
      my $t1 = build_set @{$t->[0]};
      my $t2 = new Automake::Condition @{$t->[1]};
      my $t3 = $t->[2];
      my ($ans, $cond) = $t1->ambiguous_p ("FOO", $t2);
      if ($t3 && $ans !~ /FOO.*$t3/)
	{
	  print " (A1) " . $t1->string . " vs. " . $t2->string . "\n\t"
	    . "Error message '$ans' does not match '$t3'\n";
	  return 1;
	}
      if (!$t3 && $ans ne '')
	{
	  print " (A2) " . $t1->string . " vs. " . $t2->string . "\n\t"
	    . "Unexpected error message: $ans\n";
	  return 1;
	}
    }
  return 0;
}

exit (test_basics
      || test_invert
      || test_simplify
      || test_sub_conditions
      || test_ambig);

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
