#!/usr/bin/python
#
# $Id$
#
# Read in an HTML fragment file (.hcontent) and extra a reasonable
# title for the page.
#
# python gettitle.py <sourcefile>
#
# (c) Copyright James Aylett 2009

import getopt, sys, urlparse, os
from HTMLParser import HTMLParser

def print_help():
    print "gettitle.py version 0.1 (c) James Aylett 2009"
    print
    print "Usage: %s [options] <sourcefile>" % sys.argv[0]
    print
    print "\t-v\tbe more verbose"
    print "\t-h\tprint this help"
    print

if __name__=='__main__':
    optlist, args = getopt.getopt(sys.argv[1:], 'hv', ['help', 'verbose'])

    verbose = False

    for o, p in optlist:
        if o == '-h' or o=='--help':
            print_help()
            sys.exit()
        if o == '-v' or o=='--verbose':
            verbose = True

    if len(args)!=1:
        print_help()
        sys.exit(1)
    filename = args[0]

    class MyParser(HTMLParser):
        in_heading = False
        data = None
        
        def handle_starttag(self, tag, attrs):
            # on first heading, start gobbling data
            if tag.lower() in ('h1', 'h2', 'h3', 'h4', 'h5', 'h6') and self.data==None:
                self.data = ""
                self.in_heading = True

        def handle_endtag(self, tag):
            if tag.lower() in ('h1', 'h2', 'h3', 'h4', 'h5', 'h6'):
                self.in_heading = False

        def handle_data(self, data):
            if self.in_heading:
                self.data += data

        def handle_charref(self, ref):
            if self.in_heading:
                self.data += "&#%s;" % ref

        def handle_entityref(self, name):
            if self.in_heading:
                self.data += "&%s;" % name

    try:
        parser = MyParser()
        f = open(filename, "r")
        fdata = f.read()
        f.close()
        parser.feed(fdata)
        parser.close()
        # use sys.stdout.write to avoid a trailing newline
        if parser.data:
            sys.stdout.write(parser.data)
        else:
            sys.stdout.write(os.path.basename(filename))
    except:
        # almost never happens
        sys.stdout.write("Downloads")
