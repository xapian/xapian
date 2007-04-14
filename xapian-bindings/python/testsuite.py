# Utility functions for running tests and reporting the results.
#
# Copyright (C) 2007 Lemur Consulting Ltd
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
# USA

import sys
import traceback
import xapian

class TestFail(Exception):
    pass

def expect(got, expected, message="Expected equality"):
    if got != expected:
        raise TestFail("%s: got %r, expected %r" % (message, got, expected))

def expect_query(query, expected):
    desc = query.get_description()
    if desc != expected:
        raise TestFail("Unexpected query.get_description(): got %r, expected %r" % (desc, expected))

def expect_exception(expectedclass, expectedmsg, callable, *args):
    try:
        callable(*args)
        raise TestFail("Expected %s(%r) exception" % (str(expectedclass), expectedmsg))
    except expectedclass, e:
        if str(e) != expectedmsg:
            raise TestFail("Exception string not as expected: got '%s', expected '%s'" % (str(e), expectedmsg))
        if e.__class__ != expectedclass:
            raise TestFail("Didn't get right exception class: got '%s', expected subclass '%s'" % (str(e.__class__), str(expectedclass)))


def _report_failure(msg):
    # Display the failure, with some useful context.
    import traceback, os.path
    report = []
    tb = traceback.extract_tb(sys.exc_info()[2])
    while tb[-1][2].startswith('expect'):
        tb = tb[:-1]
    filename, linenum, functionname, text = tb[-1]
    report.append("TEST FAILURE in %s: %s" % (os.path.basename(filename), msg))
    report.append("At line %d:" % linenum)
    startline = max(linenum - 5, 0)
    endline = startline + 7
    lines = open(filename).readlines()
    lines = [(linenum + 1, lines[linenum].rstrip()) for linenum in xrange(startline, endline)]
    lines = ["%4d: %s" % (linenum, line) for linenum, line in lines]
    report.extend(lines)
    report.append("Xapian version: %s" % xapian.version_string())
    try:
        import platform
        platform.system()
        platdesc = "%s %s (%s)" % platform.system_alias(platform.system(), platform.release(), platform.version())
        report.append("Platform: %s" % platdesc)
    except:
        pass

    report = '\n'.join(report)
    print >> sys.stderr, '\n' + report + '\n'
    print >> sys.stderr, 'If reporting this problem, please quote all the preceding lines from\n"TEST FAILURE" onwards.\n'


def runtest(test):
    try:
        test()
        return True
    except TestFail, e:
        _report_failure(str(e))
    except xapian.Error, e:
        _report_failure("Xapian Error: %s" % str(e))
    except KeyError:
        _report_failure("Interrupted")
    except Exception, e:
        _report_failure("%s Exception: %s" % (str(e.__class__), str(e)))
    except:
        _report_failure("Unexpected exception")
    return False

