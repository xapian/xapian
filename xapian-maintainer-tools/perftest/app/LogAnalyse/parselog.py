# Copyright (C) 2008 Lemur Consulting Ltd
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
"""parselog.py: Parse a log file.

"""

__all__ = (
    'LogParseError', 'parselog',
)

import xml.dom.minidom
from xml.dom.minidom import Node

class LogParseError(KeyError):
    """An error occurring while parsing the log file.
    """
    pass

def get_element(parent, name, required=True):
    """Get a named child node from a parent.
    
    Errors if the named node doesn't occur exactly 1 time.

    """
    nodes = parent.getElementsByTagName(name)
    if len(nodes) == 0:
        if required:
            raise LogParseError("Element %r not found", name)
        return None
    if len(nodes) > 1:
        raise LogParseError("Element %r occurred multiple times", name)
    return nodes[0]

def get_text(parent, name, required=True): 
    """Get the text from a named child node of the parent.

    Errors if the named node doesn't occur exactly 1 time.

    """
    rc = ""
    child = get_element(parent, name, required)
    if child is None:
        return None
    for node in child.childNodes:
        if node.nodeType == node.TEXT_NODE:
            rc = rc + node.data
    return rc

def get_attribute(node, name):
    return node.attributes[name].value

class MachineInfo(object):
    def __init__(self, node):
        self.physmem = get_text(node, 'physmem')

    def __str__(self):
        return "MachineInfo(physmem=%s)" % self.physmem

class IndexRun(object):
    def __init__(self, dbname, node):
        self.dbname = dbname
        self.items = []
        for item in node.getElementsByTagName("item"):
            time = float(get_text(item, "time"))
            adds = int(get_text(item, "adds"))
            freemem = int(get_text(item, "freemem", required=False))
            others = u"freemem=%d" % (
                freemem,
            )
            self.items.append((time, adds, others))
        
    def __str__(self):
        return "IndexRun(dbname=%s, %d items)" % (self.dbname, len(self.items))

class TestInfo(object):
    def __init__(self, rep_num, name, backend, node):
        self.rep_num = rep_num
        self.name = name
        self.backend = backend

        self.indexruns = []
        for indexrun in node.getElementsByTagName('indexrun'):
            dbname = get_attribute(indexrun, 'dbname')
            self.indexruns.append(IndexRun(dbname, indexrun))

    def __str__(self):
        return "TestInfo(%r, %r, %r)" % (self.rep_num, self.name, self.backend)


class ParsedLog(object):
    def __init__(self, doc):
        """Build a parsed log from a DOM document.

        """
        self.machineinfo = MachineInfo(get_element(doc, 'machineinfo'))
        tests = {}
        for rep_node in doc.getElementsByTagName("repetition"):
            rep_num = int(get_attribute(rep_node, "num"))
            for test_node in rep_node.getElementsByTagName("testcase"):
                test_name = get_attribute(test_node, "name")
                test_backend = get_attribute(test_node, "backend")
                try:
                    infos = tests[test_name]
                except KeyError:
                    infos = []
                    tests[(test_name, test_backend)] = infos
                infos.append(TestInfo(rep_num, test_name, test_backend, test_node))
                infos.sort(key=lambda x: x.rep_num)
        self.tests = {}
        for key, infos in tests.iteritems():
            infos.sort(key=lambda x: x.rep_num)
            self.tests[key] = infos

    def __str__(self):
        return "ParsedLog(%(mi)s, %(testslen)d tests)" % {
            'mi': self.machineinfo,
            'testslen': len(self.tests),
        }

def parselog(filename):
    """Parse a log file.
    
    """
    try:
        doc = xml.dom.minidom.parse(filename)
    except xml.parsers.expat.ExpatError, e:
        raise LogParseError("Invalid XML: %s" % str(e))
    result = ParsedLog(doc)
    doc.unlink()
    return result

if __name__ == '__main__':
    import sys
    if len(sys.argv) != 2:
        print "Usage: parselog.py <filename>"
        sys.exit(1)
    parsed = parselog(sys.argv[1])
    print parsed.machineinfo
    for infos in parsed.tests.itervalues():
        for info in infos:
            print "name=%s, backend=%s, rep=%s" % (
                info.name, info.backend, info.rep_num
            )
            for indexrun in info.indexruns:
                print "indexrun=%s" % indexrun.dbname
                for item in indexrun.items:
                    print "%f,%d" % (item[0], item[1])
