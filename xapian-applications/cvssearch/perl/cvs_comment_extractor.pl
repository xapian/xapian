use strict;

my $pkg = $ARGV[$0];
my $get_msg = 0;

if ($pkg ne "") {
    open (ChangeLog, "./cvs2cl.pl --stdout --xml $pkg |");
    while (<ChangeLog>){ 
        chomp;
        if (0) {
        } elsif (/<msg>/) {
            $get_msg = 1;
        } elsif (/<\/msg>/) {
            $get_msg = 0;
        } elsif ($get_msg) {
            print "$_\n";
        }
    }
    close (ChangeLog);
}
