print "Context: image/gif\n";
print "\n";
open (GIF, "<fishlogo.gif") || die "no such file exist.";
while (<GIF>) {
    print $_;
}
