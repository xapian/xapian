#-------------------------------------------------------------------
# some stuff about cvssearch
# This produces a html query interface for cvssearch
#
# Structure:
# 1. Calls $cvssearch - interface to Xapian - for query matches
# 2. grep source files for query words
# 3. sorts and rank query matches as per file
# 4. finds context matched by calling on $query
# 5. Displays results, linking them to QueryFile.cgi
#
# Author: Annie - anniec@cse.unsw.edu.au
# Date: Feb 16 2001
#-------------------------------------------------------------------

use strict;
use CGI ':all';
use Cvssearch;
use Entities;

my $ctrlA = chr(01);
my $ctrlB = chr(02);
my $ctrlC = chr(03);
my $cvsdata = Cvssearch::get_cvsdata();
my $cvsquery = "./cvsquerydb";
#-------------
# start html
#-------------

print "Content-type:text/html\n\n";
print "<html>\n";
print "<head>\n";
# ----------------------------------------
# common style sheet
# ----------------------------------------
Cvssearch::print_style_sheet();
print "</head>\n";
print "<body>\n";

#----------------------------------------
# Parse Parameters
#----------------------------------------
if (param()) {
    my $fileid = param("fileid");
    my $root = Cvssearch::sanitise_root(param("root"));
    my $pkg = param("pkg");
    my $revision = param("revision");
    my $symbol = param("symbol");
    my $filename = "";
    my $query_string = "$cvsquery $root $pkg -f $fileid -l $fileid $revision";
    my @lines;
    open (QUERY, "$query_string |");
    while (<QUERY>) {
        chomp;
        if (0) {
        } elsif (/$ctrlA/) {
            last;
        } else {
            $filename = $_;
        }
    }
    my $i = 1;
    print "<table cellspacing=\"0\" cellpadding=\"0\" width=\"100%\" border=\"0\">\n";
    open (FILE, "<$cvsdata/$root/src/$filename");
    while (<QUERY>) {
        chomp;
        if (0) {
        } elsif (/$ctrlA/) {
            last;
        } else {
            my $line_index = $_;
            my $line;
            while ($i < $line_index) {
                $line = <FILE>;
                chomp $line;
                my $space = "";
                if (length($line) == 0) {
                    $space = " ";
                }
                $line = Entities::encode_entities($line);
                if ($symbol ne "") {
                    $line = highlightquery($line, $symbol);
                }
                print "<tr><td>$i</td><td><pre>$line$space</td></tr>\n";
                $i++;
            }
            $line = <FILE>;
            chomp $line;
            my $space = "";
            if (length($line) == 0) {
                $space = " ";
            }
            $line = Entities::encode_entities($line);
            if ($symbol ne "") {
                my $old_line = $line;
                $line = highlightquery($line, $symbol);
                if ($line eq $old_line) {
                    print "<tr><td>$i </td><td><pre>$line</td></tr>\n";
                } else {
                    print "<tr><td><a name=\"$i\">$i </a></td><td class=t><pre>$line$space</td></tr>\n";
                }
            } else {
                print "<tr><td><a name=\"$i\">$i </a></td><td class=t><pre>$line$space</td></tr>\n";
            }
            $i++;
        }
    }
    close (QUERY);
    while(<FILE>) {
        chomp;
        my $line = $_;
        $line = Entities::encode_entities($line);
        print "<tr><td>$i </td><td><pre>$line</td></tr>\n";
        $i++;
    }
    close (FILE);

	print "</table>\n";	
}
print "</body></html>";

#--------------------------------
# replace digits with characters
# 0->A 1->B 2->C ..etc
#--------------------------------
sub toChar{
	my ($word) = @_;
	$word =~ s/(\d)/@{[chr($1+65)]}/ig;
	return $word;
}

#-----------------------------------
# highlightquery
# return line matched by query words
# make words matched by query bold
#-----------------------------------
sub highlightquery{
	my ($words, $word) = @_;
	$words =~ s!($word)!<b>$1</b>!ig;
	return $words;
}
