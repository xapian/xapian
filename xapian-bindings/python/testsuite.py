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

import os as _os
import os.path as _path
import sys as _sys
import traceback as _traceback
import xapian as _xapian

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

def _allow_control_sequences():
    "Return True if output device allows control sequences."
    mode = _os.environ.get("XAPIAN_TESTSUITE_OUTPUT", '').lower()
    if mode in ('', 'auto'):
        if _sys.platform == 'win32':
            return False
        elif not hasattr(_out, "isatty"):
            return False
        else:
            return _out.isatty()
    elif mode == 'plain':
        return False
    return True

def _get_colour_strings():
    """Return a mapping of colour names to colour output sequences.

    """
    colours = {
        'red': "\x1b[1m\x1b[31m",
        'green': "\x1b[1m\x1b[32m",
        'yellow': "\x1b[1m\x1b[33m",
        'reset': "\x1b[0m",
    }
    if (not _allow_control_sequences()):
        for key in colours:
            colours[key] = ''
    return colours

def _colourise(msg):
    """Apply colours to a message.

    #colourname will change the text colour, #reset will change the colour back.

    """
    msg = msg.replace('#red', _colours['red'])
    msg = msg.replace('#green', _colours['green'])
    msg = msg.replace('#yellow', _colours['yellow'])
    msg = msg.replace('##', _colours['reset'])
    return msg

def _report_failure(msg, show_traceback=True):
    "Report a test failure, with some useful context."

    orig_tb = _traceback.extract_tb(_sys.exc_info()[2])
    tb = orig_tb

    # Move up the traceback until we get to the line in the test
    # function which caused the failure.
    # FIXME - need a better way to do this.
    while tb[-1][2].startswith('expect') or \
          _os.path.basename(tb[-1][0]) == 'xapian.py':
        tb = tb[:-1]

    # Display the context in the text function.
    filepath, linenum, functionname, text = tb[-1]
    filename = _os.path.basename(filepath)

    if _verbose:
        _out.write(_colourise(" #redFAILED##\n"))
        firstline = "Failure in %s" % filename
        report = []
        report.append(firstline + ": " + msg)
        report.append("At line %d:" % linenum)
        lines = open(filepath).readlines()
        startline = max(linenum - 3, 0)
        endline = min(linenum + 2, len(lines))
        lines = [["  ", num + 1, lines[num].rstrip()]
                 for num in xrange(startline, endline)]
        lines[linenum - startline - 1][0] = "->"
        lines = ["%s%4d %s" % tuple(lineinfo) for lineinfo in lines]
        report.extend(lines)

        # Display the traceback
        if show_traceback:
            report.append("\nTraceback (most recent call last):")
            for line in _traceback.format_list(orig_tb):
                report.append(line.rstrip())
            report.append("")

        # Display some information about the xapian version and platform
        report.append("Xapian version: %s" % _xapian.version_string())
        try:
            import platform
            platdesc = "%s %s (%s)" % platform.system_alias(platform.system(),
                                                            platform.release(),
                                                            platform.version())
            report.append("Platform: %s" % platdesc)
        except:
            pass
        report = '\n' + '\n'.join(report) + '\n\nWhen reporting this problem, please quote all the preceding lines from\n"%s" onwards.\n\n' % firstline

    else:
        report = _colourise(" #redFAILED##: %s (%s, line %d)\n" % (msg, filename, linenum))

    _out.write(report)

def _runtest(name, test_fn):
    """Run a single test.
H

    """
    startline = "Running test: %s..." % name
    _out.write(startline)
    _out.flush()
    try:
        test_fn()
        if _verbose:
            _out.write(_colourise(" #greenok##\n"))
        else:
            if (_allow_control_sequences()):
                _out.write(_colourise("\r" + " " * len(startline) + "\r"))
            else:
                _out.write('\n')
        return True
    except TestFail, e:
        _report_failure(str(e), show_traceback=False)
    except _xapian.Error, e:
        _report_failure("%s: %s" % (str(e.__class__), str(e)))
    except Exception, e:
        _report_failure("%s: %s" % (str(e.__class__), str(e)))
    except:
        _report_failure("Unexpected exception")
    return False

def runtests(namedict):
    """Run a set of tests.
    
    Takes a dictionary of name-value pairs and runs all the values which are
    callables, for which the name begins with "test_".

    Typical usage is to pass "locals()" as the parameter, to run all callables
    with names starting "test_" in local scope.

    """
    tests = []
    for name in namedict:
        if name.startswith('test_'):
            fn = namedict[name]
            name = name[5:]
            if callable(fn):
                tests.append((name, fn))

    passed, failed = 0, 0
    for name, fn in tests:
        if _runtest(name, fn):
            passed += 1
        else:
            failed += 1
    if failed:
        if not _verbose:
            print 'Re-run with the VERBOSE environment variable set to "1" to see details.'
        print _colourise("#green%d## tests passed, #red%d## tests failed") % (passed, failed)
        return False
    else:
        print _colourise("#green%d## tests passed, no failures") % passed
        return True


_out = _sys.stdout
_colours = _get_colour_strings()
_verbose = _os.environ.get('VERBOSE', '').lower()
if _verbose in ('', '0', 'no', 'off', 'false'):
    _verbose = False
else:
    _verbose = True
