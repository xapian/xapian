#!/usr/bin/python
#
# $Id$
#
# Read in a template file, performing substitutions for {...}.
#
# python genhtml.py <template> <input> [NAME=VALUE ...]
#
# (c) Copyright James Aylett 2008

import sys, os, getopt

def print_help():
    print "genhtml.py version 0.2 (c) James Aylett 2008"
    print
    print "Usage: %s [options] <template> <input> [NAME=VALUE ...]" % sys.argv[0]
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

    if len(args)<2:
        print_help()
        sys.exit(1)

    replace = { '$<' : args[1] }
    for b in args[2:]:
        b = b.split('=')
        if len(b)!=2:
            print_help()
            sys.exit(1)
        replace['$(' + b[0] + ')'] = b[1]

    f = open(args[0])
    for line in f.readlines():
        for k in replace:
            line = line.replace(k, replace[k])
        opos = line.find('{')
        cpos = line.find('}')
        while opos!=-1 and cpos!=-1:
            cmd = line[opos+1:cpos].strip()
            if verbose:
                sys.stderr.write("Running `%s`\n" % cmd)
            p = os.popen(cmd, 'r')
            repl = p.read()
            r = p.close()
            if r!=None:
                sys.stderr.write("Error running `%s`: %i\n" % (cmd, r))
                sys.exit(1)
            line = line[0:opos] + repl + line[cpos+1:]
            nxt = opos + len(repl)
            opos = line.find('{', nxt)
            cpos = line.find('}', nxt)

        sys.stdout.write(line)
    f.close()
