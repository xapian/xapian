print "Context: image/png\n\n";
open (GIF, "<fishlogo.png") || die "no such file exist.";
while (<GIF>) {
    print $_;
}
