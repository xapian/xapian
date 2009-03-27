# Utility functions for running tests and reporting the results.
#
# Copyright (C) 2007 Lemur Consulting Ltd
# Copyright (C) 2008 Olly Betts
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

class TestRunner(object):
    def __init__(self):
        """Initialise the TestRunner.

        """

        self._out = OutProxy(_sys.stdout)

        # _verbose is an integer, higher meaning more verbose
        self._verbose = _os.environ.get('VERBOSE', '').lower()
        if self._verbose in ('', '0', 'no', 'off', 'false'):
            self._verbose = 0
        else:
            try:
                self._verbose = int(self._verbose)
            except:
                self._verbose = 1

        # context is a description of what the test is currently checking
        self._context = None

    def context(self, context):
        """Set the context.

        This should be a string describing what a test is checking, and will be
        displayed if the test fails.

        A test may change the context several times - each call will override
        subsequent calls.

        Set the context to None to remove display of a specific context message.
        This is performed automatically at the start of each test.

        """
        self._context = context
        if context is not None and self._verbose > 1:
            self._out.start_line()
            self._out.write("Context: %s\n" % context)
            self._out.flush()

    def expect(self, got, expected, message="Expected equality"):
        """Function used to check for a particular expected value.

        """
        if self._verbose > 2:
            self._out.start_line()
            self._out.write("Checking for %r: expecting %r ... " % (message, expected))
            self._out.flush()
        if got != expected:
            if self._verbose > 2:
                self._out.write_colour(" #red#failed##")
                self._out.write(": got %r\n" % got)
                self._out.flush()
            raise TestFail("%s: got %r, expected %r" % (message, got, expected))
        if self._verbose > 2:
            self._out.write_colour(" #green#ok##\n")
            self._out.flush()

    def expect_query(self, query, expected):
        """Check that the description of a query is as expected.

        """
        expected = 'Xapian::Query(' + expected + ')'
        desc = str(query)
        if self._verbose > 2:
            self._out.start_line()
            self._out.write("Checking str(query): expecting %r ... " % expected)
            self._out.flush()
        if desc != expected:
            if self._verbose > 2:
                self._out.write_colour(" #red#failed##")
                self._out.write(": got %r\n" % desc)
                self._out.flush()
            raise TestFail("Unexpected str(query): got %r, expected %r" % (desc, expected))
        if self._verbose > 2:
            self._out.write_colour(" #green#ok##\n")
            self._out.flush()

    def expect_exception(self, expectedclass, expectedmsg, callable, *args):
        if self._verbose > 2:
            self._out.start_line()
            self._out.write("Checking for exception: %s(%r) ... " % (str(expectedclass), expectedmsg))
            self._out.flush()
        try:
            callable(*args)
            if self._verbose > 2:
                self._out.write_colour(" #red#failed##: no exception occurred\n")
                self._out.flush()
            raise TestFail("Expected %s(%r) exception" % (str(expectedclass), expectedmsg))
        except expectedclass as e:
            if str(e) != expectedmsg:
                if self._verbose > 2:
                    self._out.write_colour(" #red#failed##")
                    self._out.write(": exception string not as expected: got '%s'\n" % str(e))
                    self._out.flush()
                raise TestFail("Exception string not as expected: got '%s', expected '%s'" % (str(e), expectedmsg))
            if e.__class__ != expectedclass:
                if self._verbose > 2:
                    self._out.write_colour(" #red#failed##")
                    self._out.write(": didn't get right exception class: got '%s'\n" % str(e.__class__))
                    self._out.flush()
                raise TestFail("Didn't get right exception class: got '%s', expected '%s'" % (str(e.__class__), str(expectedclass)))
        if self._verbose > 2:
            self._out.write_colour(" #green#ok##\n")
            self._out.flush()

    def report_failure(self, name, msg, show_traceback=True):
        "Report a test failure, with some useful context."

        orig_tb = _traceback.extract_tb(_sys.exc_info()[2])
        tb = orig_tb

        # Move up the traceback until we get to the line in the test
        # function which caused the failure.
        while tb[-1][2] != 'test_' + name:
            tb = tb[:-1]

        # Display the context in the text function.
        filepath, linenum, functionname, text = tb[-1]
        filename = _os.path.basename(filepath)

        self._out.ensure_space()
        self._out.write_colour("#red#FAILED##\n")
        if self._verbose > 0:
            if self._context is None:
                context = ''
            else:
                context = ", when %s" % self._context
            firstline = "%s:%d" % (filename, linenum)
            self._out.write("\n%s:%s%s\n" % (firstline, msg, context))

            # Display sourcecode lines
            lines = open(filepath).readlines()
            startline = max(linenum - 3, 0)
            endline = min(linenum + 2, len(lines))
            for num in range(startline, endline):
                if num + 1 == linenum:
                    self._out.write('->')
                else:
                    self._out.write('  ')
                self._out.write("%4d %s\n" % (num + 1, lines[num].rstrip()))

            # Display the traceback
            if show_traceback:
                self._out.write("Traceback (most recent call last):\n")
                for line in _traceback.format_list(orig_tb):
                    self._out.write(line.rstrip() + '\n')
                self._out.write('\n')

            # Display some information about the xapian version and platform
            self._out.write("Xapian version: %s\n" % _xapian.version_string())
            try:
                import platform
                platdesc = "%s %s (%s)" % platform.system_alias(platform.system(),
                                                                platform.release(),
                                                                platform.version())
                self._out.write("Platform: %s\n" % platdesc)
            except:
                pass
            self._out.write('\nWhen reporting this problem, please quote all the preceding lines from\n"%s" onwards.\n\n' % firstline)

        self._out.flush()

    def runtest(self, name, test_fn):
        """Run a single test.
    
        """
        startline = "Running test: %s..." % name
        self._out.write(startline)
        self._out.flush()
        try:
            test_fn()
            if self._verbose > 0 or self._out.plain:
                self._out.ensure_space()
                self._out.write_colour("#green#ok##\n")
            else:
                self._out.clear_line()
            self._out.flush()
            return True
        except TestFail as e:
            self.report_failure(name, str(e), show_traceback=False)
        except _xapian.Error as e:
            self.report_failure(name, "%s: %s" % (str(e.__class__), str(e)))
        except Exception as e:
            self.report_failure(name, "%s: %s" % (str(e.__class__), str(e)))
        return False

    def runtests(self, namedict, runonly=None):
        """Run a set of tests.
    
        Takes a dictionary of name-value pairs and runs all the values which are
        callables, for which the name begins with "test_".

        Typical usage is to pass "locals()" as the parameter, to run all callables
        with names starting "test_" in local scope.

        If runonly is supplied, and non-empty, only those tests which appear in
        runonly will be run.

        """
        tests = []
        if isinstance(namedict, dict):
            for name in namedict:
                if name.startswith('test_'):
                    fn = namedict[name]
                    name = name[5:]
                    if hasattr(fn, '__call__'):
                        tests.append((name, fn))
            tests.sort()
        else:
            tests = namedict

        if runonly is not None and len(runonly) != 0:
            oldtests = tests
            tests = []
            for name, fn in oldtests:
                if name in runonly:
                    tests.append((name, fn))

        passed, failed = 0, 0
        for name, fn in tests:
            self.context(None)
            if self.runtest(name, fn):
                passed += 1
            else:
                failed += 1
        if failed:
            if self._verbose == 0:
                self._out.write('Re-run with the VERBOSE environment variable set to "1" to see details.\n')
            self._out.write_colour("#green#%d## tests passed, #red#%d## tests failed\n" % (passed, failed))
            return False
        else:
            self._out.write_colour("#green#%d## tests passed, no failures\n" % passed)
            return True

class OutProxy(object):
    """Proxy output class to make formatting easier.

    Allows colourisation, and keeps track of whether we're mid-line or not.

    """

    def __init__(self, out):
        self._out = out
        self._line_pos = 0 # Position on current line
        self._had_space = True # True iff we're preceded by whitespace (including newline)
        self.plain = not self._allow_control_sequences()
        self._colours = self.get_colour_strings()

    def _allow_control_sequences(self):
        "Return True if output device allows control sequences."
        mode = _os.environ.get("XAPIAN_TESTSUITE_OUTPUT", '').lower()
        if mode in ('', 'auto'):
            if _sys.platform == 'win32':
                return False
            elif not hasattr(self._out, "isatty"):
                return False
            else:
                return self._out.isatty()
        elif mode == 'plain':
            return False
        return True

    def get_colour_strings(self):
        """Return a mapping of colour names to colour output sequences.

        """
        colours = {
            'red': "\x1b[1m\x1b[31m",
            'green': "\x1b[1m\x1b[32m",
            'yellow': "\x1b[1m\x1b[33m",
            '': "\x1b[0m",
        }
        if self.plain:
            for key in colours:
                colours[key] = ''
        return colours

    def _colourise(self, msg):
        """Apply colours to a message.

        #colourname# will change the text colour, ## will change the colour back.

        """
        for colour, val in self._colours.items():
            msg = msg.replace('#%s#' % colour, val)
        return msg

    def clear_line(self):
        """Clear the current line of output, if possible.

        Otherwise, just move to the start of the next line.

        """
        if self._line_pos == 0:
            return
        if self.plain:
            self.write('\n')
        else:
            self.write("\r" + " " * self._line_pos + "\r")

    def start_line(self):
        """Ensure that we're at the start of a line.

        """
        if self._line_pos != 0:
            self.write('\n')

    def ensure_space(self):
        """Ensure that we're preceded by whitespace.

        """
        if not self._had_space:
            self.write(' ')

    def write(self, msg):
        """Write the message to the output stream.

        """
        if len(msg) == 0:
            return

        # Adjust the line position counted
        nlpos = max(msg.rfind('\n'), msg.rfind('\r'))
        if nlpos >= 0:
            subline = msg[nlpos + 1:]
            self._line_pos = len(subline) # Note - doesn't cope with tabs.
        else:
            self._line_pos += len(msg) # Note - doesn't cope with tabs.

        # Record whether we ended with whitespace
        self._had_space = msg[-1].isspace()

        self._out.write(msg)

    def write_colour(self, msg):
        """Write a message, first substituting markup for colours.

        """
        self.write(self._colourise(msg))

    def flush(self):
        self._out.flush()


_runner = TestRunner()
context = _runner.context
expect = _runner.expect
expect_query = _runner.expect_query
expect_exception = _runner.expect_exception
runtests = _runner.runtests

__all__ = ('context', 'expect', 'expect_query', 'expect_exception', 'runtests')
