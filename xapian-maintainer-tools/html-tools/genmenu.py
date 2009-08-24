#!/usr/bin/python
#
# Read in an XHTML fragment file containing a <ul> element to be
# treated as a menu, and spit it out with the 'selected' class applied
# to the relevant <li> element.
#
# "Local menu" means it's the docs menu (basically) rather than the
# global menu (About | Support etc).
# "Local build" means it's being built for distribution rather than
# for the website.
#
# python genmenu.py <menufile> <source-path-to-match> [uriroot]
#
# (c) Copyright James Aylett 2008, 2009

import getopt, sys, urlparse
import xml.dom, xml.dom.minidom

def check_prefix(localpath, menupath, localmenu=False, localroot=None):
    # menupath won't contain any suffix; localpath will.
    # Only support the ones we actually use.
    # localroot is *only* used to generate the bindings, where the source
    # storage is utterly different to the destination. Fix things up here.
    if localmenu and localroot:
        menupath = menupath + "docs/"
    if menupath[-1]!='/':
        if localpath.endswith('.md'):
            menupath = menupath + '.md'
        elif localpath.endswith('.hcontent'):
            menupath = menupath + '.hcontent'
    elif localmenu:
        menupath = menupath + 'index.md'
    #print "checking %s vs %s [%i]" % (localpath, menupath, localmenu)
    # Really, the following does work. The local menu is always
    # specific articles, but the top menu is categories.
    # (You need to ensure that you pass in the source *URI*
    # for the top menu rather than the source filename.)
    #sys.stderr.write("check: %s, %s, %s, %s\n" % (localpath, menupath, localmenu, localroot,))
    if localmenu:
        #return localpath.endswith(menupath)
        return localpath == menupath
    else:
        return localpath.startswith(menupath)

def process(node, localpath, uriroot=None, verbose=False, localmenu=False, localbuild=False, localroot=None):
    if node.nodeType==node.ELEMENT_NODE and node.nodeName=='li':
        for child in node.childNodes:
            if child.nodeType==child.ELEMENT_NODE and child.nodeName=='a':
                if verbose:
                    sys.stderr.write("li a[href=%s] << %s\n" % (child.getAttribute('href'), localpath))
                if check_prefix(localpath, child.getAttribute('href'), localmenu, localroot):
                    classes = node.getAttribute('class')
                    if classes!='':
                        classes += ' '
                    classes += 'selected'
                    node.setAttribute('class', classes)
    if node.nodeType==node.ELEMENT_NODE and node.nodeName=='a':
        href = node.getAttribute('href') # if nothing, returns ''
        if href!='' and uriroot!=None:
            href = urlparse.urljoin(uriroot, href)
        if localbuild and localmenu and href[-1]!='/':
            href += '.html'
        if localbuild and localmenu and localroot!=None:
            href = urlparse.urljoin(localroot, href)
        if verbose:
            sys.stderr.write("a[href=%s] << %s << %s [*%s / %s*]\n" % (href, uriroot, node.getAttribute('href'), localmenu, localbuild))
        if not localbuild and uriroot==None:
            nlevels = localpath.count('/')
            href = urlparse.urljoin('../' * nlevels, href)
        node.setAttribute('href', href)
    for child in node.childNodes:
        process(child, localpath, uriroot, verbose, localmenu, localbuild, localroot)

def print_help():
    print "genmenu.py version 0.5 (c) James Aylett 2009"
    print
    print "Usage: %s [options] <menufile> <URI-path-to-match> [uriroot]" % sys.argv[0]
    print
    print "\t-v\tbe more verbose"
    print "\t-l\tbuild local menu"
    print "\t-r\troot for local menu"
    print "\t-b\tbuild for local documentation"
    print "\t-h\tprint this help"
    print

if __name__=='__main__':
    optlist, args = getopt.getopt(sys.argv[1:], 'hblr:v', ['help', 'localbuild', 'localmenu', 'localroot=', 'verbose'])

    verbose = False
    localmenu = False
    localbuild = False
    localroot = None

    for o, p in optlist:
        if o == '-h' or o=='--help':
            print_help()
            sys.exit()
        if o == '-l' or o=='--localmenu':
            localmenu = True
        if o == '-b' or o=='--localbuild':
            localbuild = True
        if o == '-r' or o=='--localroot':
            localroot = p
        if o == '-v' or o=='--verbose':
            verbose = True

    if len(args)<2 or len(args)>3:
        print_help()
        sys.exit(1)
    filename = args[0]
    localpath = args[1]
    if len(args)==2:
        uriroot = None # '/'
    else:
        uriroot = args[2]
        if uriroot=='':
            uriroot=None

    if localmenu:
        uriroot=None

    dom = xml.dom.minidom.parse(filename)
    process(dom, localpath, uriroot, verbose, localmenu, localbuild, localroot)
    out = dom.toxml('utf-8')
    outl = out.split('\n')
    XML_PREAMBLE = '<?xml version="1.0" encoding="utf-8"?>'
    if outl[0] == XML_PREAMBLE:
        out = '\n'.join(outl[1:]) # get rid of XML preamble
    elif outl[0].startswith(XML_PREAMBLE):
        outl[0] = outl[0][len(XML_PREAMBLE):]
        out = '\n'.join(outl)
    else:
        out = '\n'.join(outl)
    print out
