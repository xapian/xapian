use strict;

my $line;
my $temp;
for (my $i = 0; $i <= $#ARGV; $i++) {
    open (FILE, $ARGV[$i]);
    while (<FILE>) {
        chomp;
        $line = $_;
        if (m/\/\/(.*)/) {
            print "$1\n";
        }
        $temp .= $line;
        $temp .= " ";
    }
    close(FILE);
    while ($temp =~ s#(/\*(.*?)\*/)##) {
           print "$1\n";
       }
}
