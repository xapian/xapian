use strict;

my $pwd = $ARGV[0];
my $path= $ARGV[1];
my $pkg = $ARGV[2];
my $get_msg = 0;

if ($pkg ne "") {
    chdir $path || die "cannot change directory to $path: $!";
    open (ChangeLog, "$pwd/cvs2cl --stdout --xml $pkg |");
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
    chdir $pwd || die "cannot change directory to $pwd: $!";
}
