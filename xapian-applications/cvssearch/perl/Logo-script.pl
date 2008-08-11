use strict;

print "Content-Type: image/png\n\n";
open IMG, "<fishlogo.png" or die "Couldn't read image: $!\n";
while (<IMG>) {
    print $_;
}
