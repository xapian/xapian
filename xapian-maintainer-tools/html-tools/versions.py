#!/usr/bin/python
#
# $Id$
#
# Replace various @@<foo>@@, used principally for the download page.
#
# python versions.py <input>

import time, datetime

# Update this on releases.
replace = {
    '@@VERSION@@': '1.0.14',
    '@@BRANCH@@': '1.0',
    '@@DATE@@': '2009-07-21',
    '@@PERL_MINOR@@': '0',
}

# Auto-generate some replacement stuff

# Where do we find Perl in the SVN repo?
if replace['@@PERL_MINOR@@'] == '0':
    replace['@@PERL_TAG@@'] = '%s/search-xapian' % replace['@@VERSION@@']
else:
    replace['@@PERL_TAG@@'] = 'search-xapian-%s.%s' % (
        replace['@@VERSION@@'],
        replace['@@PERL_MINOR@@'],
    )

# Should we warn that CPAN may be out of date?
# We do so in the first week after a release.
t = time.strptime(replace['@@DATE@@'], '%Y-%m-%d')
dt1 = datetime.date(t[0], t[1], t[2])
tn = time.gmtime()
dt2 = datetime.date(tn[0], tn[1], tn[2])
if (dt2 - dt1).days < 7:
    replace['@@CPAN_WARNING@@'] = '<small>(CPAN mirrors may not have updated yet)</small>'
else:
    replace['@@CPAN_WARNING@@'] = ''

import sys, getopt

def print_help():
    print "versions.py (Xapian version insanitiser)"
    print
    print "Usage: %s [options] <input>" % sys.argv[0]
    print
    print "\t-v\tbe more verbose"
    print "\t-h\tprint this help"
    print

if __name__=="__main__":
    optlist, args = getopt.getopt(sys.argv[1:], 'hv', ['help', 'verbose'])

    verbose = False

    for o, p in optlist:
        if o == '-h' or o=='--help':
            print_help()
            sys.exit()
        if o == '-v' or o=='--verbose':
            verbose = True

    if len(args)<1:
        print_help()
        sys.exit(1)

    f = open(args[0])
    for line in f.readlines():
        for k in replace:
            line = line.replace(k, replace[k])
        sys.stdout.write(line)
    f.close()
