#!/usr/bin/env python
#
# Copyright (C) 2007 Lemur Consulting Ltd
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
r"""audit.py: Simple script to check code ownership and license messages.

Currently assumes that the xapian code can be found in "../xapian-core".

"""

import csv
import re
import os, os.path
from pprint import pprint
import sys

copy_re = re.compile(r'Copyright\s+(\([Cc]\))?\s*(?P<dates>([0-9]{2,4})((,\s*|-)[0-9]{2,4})*),?\s*$')
copy2_re = re.compile(r'Copyright\s+(\([Cc]\))?\s*(?P<dates>([0-9]{2,4})((,\s*|-)[0-9]{2,4})*),?\s+(?P<name>.+)\s*$')
copy_unrec_re = re.compile(r'Copyright')

directive_re = re.compile(r'\s*#\s*error')

# Copyright holders which mean code is GPL only.
gplonly = [
    'BrightStation PLC',
    'Ananova Ltd',
]

licenses = [
    ('lgpl2+', r'''
     is free software; you can redistribute it and\/or modify it under the
     terms of the GNU Library General Public License as published by the Free
     Software Foundation; either version 2 of the License, or \(at your option\)
     any later version.
     '''),
    ('gpl2+', r'''
     is free software; you can redistribute it and\/or modify it under the terms of
     the GNU General Public License as published by the Free Software Foundation;
     either version 2( of the License)?, or \(at your option\) any later version.
     '''),
    ('sgi-historical', r'''
    Permission to use, copy, modify, distribute and sell this software and its
    documentation for any purpose is hereby granted without fee, provided that
    the above copyright notice appear in all copies and that both that
    copyright notice and this permission notice appear in supporting
    documentation.
    '''),
    ('pub_domain', r'''
    The authors of this program disclaim copyright.
    '''),
    ('mit_x', r'''
    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files \(the "Software"\),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:
    '''),
]

fixmes = [
    r'''FIXME:(?P<milestone>[\d.]+)''',
    r'''FIXME''',
]

whitespace_re = re.compile(r'\s+')

license_patterns = []
for name, pattern in licenses:
    pattern = whitespace_re.sub('\s+', pattern)
    license_patterns.append((name, re.compile(pattern)))

fixme_patterns = []
for pattern in fixmes:
    fixme_patterns.append(re.compile(pattern))

class FileDetails:
    def __init__(self, path):
        self.path = path
        self.holders = []
        self.licenses = []
        self.length = 0
        self.fixmes = []

    def __repr__(self):
        return "FileDetails(%r, %r, %r, %r)" % (self.path, self.holders, self.licenses, self.fixmes)

class SourceChecker:
    def __init__(self, toppath):
        self.files = {}

        self.toppath = os.path.normpath(os.path.abspath(toppath))
        if os.path.isdir(self.toppath):
            self.topdirpath = self.toppath
        else:
            self.topdirpath = os.path.dirname(self.toppath)
        self.current_path = None

    def warn(self, msg):
        print("Warning in %s: %s" % (self.current_path, msg))

    def get_file_details(self, path=None):
        if path is None:
            path = self.current_path
        try:
            return self.files[path]
        except KeyError:
            details = FileDetails(path)
            self.files[path] = details
            return details

    def parse_date_list(self, dates):
        newdates = []
        prevdate = None
        for date in dates.split(','):
            if '-' in date:
                begin, end = date.split('-')
                begin = int(begin)
                end = int(end)
                if end < begin:
                    self.warn('Invalid date range %r in copyright' % date)
                    newdates.append(begin)
                for date in xrange(begin, end + 1):
                    newdates.append(date)
                prevdate = end
            else:
                date = int(date)
                if date < 1000:
                    if prevdate is None or date >= 100:
                        self.warn('Invalid date %r in copyright' % date)
                    else:
                        date = (prevdate // 100) * 100 + date
                newdates.append(int(date))
                prevdate = date
        return newdates

    def add_copyright_holder(self, name, dates):
        file = self.get_file_details()

        dates = self.parse_date_list(dates)

        file.holders.append((name, dates))

    def parse_copyrights(self, comments):
        seen_copyright = False
        dates = None
        got_date_line = False
        for comment in comments:
            for line in comment.split('\n'):
                if got_date_line:
                    self.add_copyright_holder(line, dates)
                    got_date_line = False

                m = copy_re.search(line)
                m2 = copy2_re.search(line)
                if m:
                    dates = m.group('dates')
                    got_date_line = True
                elif m2:
                    name = m2.group('name')
                    dates = m2.group('dates')
                    self.add_copyright_holder(name, dates)
                    seen_copyright = True
                elif copy_unrec_re.search(line):
                    self.warn("Unrecognised copyright line: %r" % line)

    def parse_licenses(self, comments):
        licenses = []
        for comment in comments:
            comment = comment.replace('\n', ' ').replace('\r', '').strip()
            for license, pattern in license_patterns:
                if pattern.search(comment):
                    licenses.append(license)
        if len(licenses) == 0:
            self.warn("No license found: %s" % self.current_path)

        file = self.get_file_details()
        file.licenses.extend(licenses)

    def parse_fixmes(self, comments):
        fixmes = []
        for comment in comments:
            comment = comment.replace('\n', ' ').replace('\r', '').strip()
            for pattern in fixme_patterns:
                g = pattern.search(comment)
                if g:
                    fixmetext = comment[g.end():].strip()
                    if fixmetext.startswith(':'):
                        fixmetext = fixmetext[1:].strip()
                    if fixmetext.startswith('-'):
                        fixmetext = fixmetext[1:].strip()
                    try:
                        milestone = g.group('milestone')
                    except IndexError:
                        milestone = ''
                    fixmes.append((milestone, fixmetext))
                    break

        file = self.get_file_details()
        file.fixmes.extend(fixmes)

    def strip_quotes(self, line, incomment, was_cpp_comment):
        """Remove any quoted strings from a line.

        """
        if incomment is not None:
            if was_cpp_comment:
                incomment = False
            else:
                incomment = True

        pos = 0
        in_quote = False
        while pos < len(line):
            if incomment:
                if pos + 1 < len(line) and line[pos:pos+2] == '*/':
                    pos += 2
                    incomment = False
                    continue
                else:
                    pos += 1
                    continue

            if not incomment and not in_quote:
                if pos + 1 < len(line):
                    if line[pos:pos+2] == '/*':
                        pos += 2
                        incomment = True
                        continue
                    if line[pos:pos+2] == '//':
                        break

            if not in_quote:
                if line[pos] == "'":
                    start = pos
                    try:
                        pos += 1
                        if line[pos] == '\\':
                            pos += 1
                            if line[pos] == 'x':
                                pos += 2
                        pos += 1
                        if line[pos] != "'":
                            self.warn("Unmatched single quote: %r" % line)
                            pos = start + 1
                            continue
                        else:
                            line = line[:start] + line[pos+1:]
                            pos = start
                            continue
                    except IndexError:
                        self.warn("Unfinished single quote: %r" % line)
                        return line

                if line[pos] == '"':
                    start = pos
                    in_quote = True
            else:
                if line[pos] == '\\':
                    pos += 2
                    if pos >= len(line):
                        self.warn("Unfinished double quote: %r" % line)
                        return line
                    continue
                if line[pos] == '"':
                    in_quote = False
                    line = line[:start] + line[pos+1:]
                    pos = start
                    continue

            pos += 1
        return line

    def strip_directives(self, line):
        if directive_re.match(line):
            return ''
        return line

    def join_slashed_lines(self, lines):
        "Join lines terminated with \ together"
        newlines = []
        had_slash = False
        for line in lines:
            if had_slash:
                newlines[-1] += line
            else:
                newlines.append(line)

            had_slash = False
            if line.endswith('\\'):
                had_slash = True
                newlines[-1] = newlines[-1][:-1]
        return newlines

    def get_comments(self, lines):
        """Get the C or C++ style comments from a set of lines.

        """
        comments = []
        incomment = None
        was_cpp_comment = False
        lines = self.join_slashed_lines(lines)

        for line in lines:
            line = line.strip()
            if len(line) == 0:
                continue
            line = self.strip_directives(line)
            line = self.strip_quotes(line, incomment, was_cpp_comment)
            pos = 0
            if incomment is not None:
                if not was_cpp_comment:
                    # Look for the end of a C comment
                    end = line.find('*/', 0)

                    # Check for leading "*"s
                    if end != 0 and line[0] == '*':
                        line = line[1:].strip()
                        end -= 1

                    # End the comment if an end was found
                    if len(incomment) != 0 and incomment[-1] != '\n':
                        incomment += '\n'
                    if end >= 0:
                        pos = end + 2
                        incomment += line[:end]
                        comments.append(incomment)
                        incomment = None
                    else:
                        incomment += line

                if was_cpp_comment:
                    # Look for a continuation C++ comment at the start of the line.
                    cpp_start = line.find('//', 0)
                    if cpp_start == 0:
                        incomment += '\n'
                        incomment += line[2:]
                    else:
                        comments.append(incomment)
                        incomment = None

            if incomment is None:
                # Look for the start of a comment
                cc_start = line.find('/*', pos)
                while cc_start != -1:
                    if line[cc_start] == '*' and line[cc_start+1] != '/':
                        # Skip extra * at start of comment, indicating a
                        # doccommment.
                        cc_start += 1
                    end = line.find('*/', cc_start+1)
                    if end == -1:
                        incomment = line[cc_start + 2:]
                        was_cpp_comment = False
                        break
                    pos = end + 2
                    comments.append(line[cc_start + 2:end])
                    cc_start = line.find('/*', pos)

            if incomment is None:
                # Look for the start of a C++ comment
                cpp_start = line.find('//', pos)
                if cpp_start != -1:
                    incomment = line[cpp_start + 2:]
                    was_cpp_comment = True

        if incomment:
            comments.append(incomment)
        return comments


    def check_file(self, path):
        '''Check the copyright status of a file.

        Returns a tuple of form (name, (year, year,))

        '''
        fd = open(path)
        lines = [line.strip() for line in fd.readlines()]
        assert(path.startswith(self.topdirpath))
        self.current_path = path[len(self.topdirpath) + 1:]

        comments = self.get_comments(lines)
        self.parse_copyrights(comments)
        self.parse_licenses(comments)
        self.parse_fixmes(comments)

        file = self.get_file_details()
        file.length = len(lines)

    def check(self):
        if os.path.isdir(self.toppath):
            for dirpath, dirnames, filenames in os.walk(self.toppath):
                for filename in filenames:
                    if filename.endswith('.cc') or \
                       filename.endswith('.c') or \
                       filename.endswith('.h'):
                        path = os.path.join(dirpath, filename)
                        self.check_file(path)
        else:
            self.check_file(self.toppath)

    def get_relicense_classses(self):
        classes = {}
        for path, details in self.files.iteritems():
            if 'gpl2+' not in details.licenses:
                classes.setdefault('nongpl', []).append(path)
                continue
            cls = 'gpl'
            holders = [item[0] for item in details.holders]
            for holder in gplonly:
                if holder in holders:
                    cls = 'gplonly'
                    break
            classes.setdefault(cls, []).append(path)
        return classes

    def get_ownership(self):
        """Get a dict holding ownership, keyed by copyright holder.

        The values are tuples, (number of files, sum of proportion of files
        held, sum of proportion weighted by number of years of files held)

        """
        # Get a dictionary, keyed by license, holding dictionaries keyed by
        # copyright holder, holding a list of values representing the
        # contribution of that holder.
        owners = {}
        for file in self.files.itervalues():
            file_ownership = {}
            holder_count = len(file.holders)
            holder_date_count = 0
            for holder_name, holder_dates in file.holders:
                holder_date_count += len(holder_dates)
            for holder_name, holder_dates in file.holders:
                proportion_equal = float(1)/holder_count
                proportion_date = float(len(holder_dates)) / holder_date_count
                file_ownership[holder_name] = [1, file.length,
                                               proportion_equal,
                                               proportion_date,
                                               proportion_equal * file.length,
                                               proportion_date * file.length,]
            
            for license in file.licenses:
                try:
                    license_owners = owners[license]
                except KeyError:
                    license_owners = {}
                    owners[license] = license_owners

                for holder_name, holder_values in file_ownership.iteritems():
                    try:
                        license_owner = license_owners[holder_name]
                    except KeyError:
                        license_owner = [0] * len(holder_values)
                        license_owners[holder_name] = license_owner
                    for i in xrange(len(holder_values)):
                        license_owner[i] += holder_values[i]

        # Get a list of the total number of lines for each license, and sort
        # into descending order.
        license_total_lines = []
        for license, owner in owners.iteritems():
            total_lines = 0
            for holder_values in owner.itervalues():
                total_lines += holder_values[4]
            license_total_lines.append((total_lines, license))
        license_total_lines.sort()
        license_total_lines.reverse()

        # Get a list of the contributors for each license, in descending order of total number of lines
        result = []
        for total_lines, license in license_total_lines:
            license_owners = []
            for owner, values in owners[license].iteritems():
                item = [owner]
                item.extend(values)
                license_owners.append(tuple(item))
            license_owners.sort(cmp=lambda x,y:cmp(x[1],y[1]))
            license_owners.reverse()
            result.append((license, license_owners))
        return tuple(result)
                
    def get_fixmes(self):
        """Get a dict holding fixmes, keyed by milestone.

        """
        milestones = {}
        for file in self.files.itervalues():
            for milestone, fixmetext in file.fixmes:
                if milestone not in milestones:
                    milestones[milestone] = []
                milestones[milestone].append((file.path, fixmetext))
        def cmpfn(a, b):
            if (a[0] == '') ^ (b[0] == ''):
                return -cmp(a, b)
            return cmp(a, b)
        return sorted([(milestone, sorted(milestones[milestone]))
                      for milestone in milestones.iterkeys()],
                      cmp=cmpfn)


toppath = '../xapian-core'
if len(sys.argv) > 1:
    toppath = sys.argv[1]
checker = SourceChecker(toppath)
checker.check()

#pprint(checker.files)

#pprint(checker.get_fixmes())
fixmefd = open("fixmes.csv", "wb")
writer = csv.writer(fixmefd)
writer.writerow(("Milestone", "File", "Message",))
for milestone, fixmes in checker.get_fixmes():
    for filepath, fixmetext in fixmes:
       writer.writerow((milestone, filepath, fixmetext))
fixmefd.close()


#pprint(checker.get_ownership())

copyrightfd = open("copyright.csv", "wb")
writer = csv.writer(copyrightfd)
writer.writerow(("License", "Author", "File count", "Lines touched",
                 "File proportion (equal)", "File proportion (biased)",
                 "Lines proportion (equal)", "Lines proportion (biased)",))
for license in checker.get_ownership():
    for holder in license[1]:
        value = [license[0]]
        value.extend(holder)
        writer.writerow(value)
copyrightfd.close()

relicense_classes = checker.get_relicense_classses()
print ('%d files:' % len(checker.files))
print ('%d files "tainted" by unrelicensable GPL code' %
       len(relicense_classes.get('gplonly', ())))
print ('%d files "tainted" by relicensable GPL code' %
       len(relicense_classes.get('gpl', ())))
print ('%d files "untainted" by GPL code' %
       len(relicense_classes.get('nongpl', ())))

fd = open("license_classes.csv", "wb")
writer = csv.writer(fd)
writer.writerow(("Status", "File path"))
for cls, paths in sorted(relicense_classes.iteritems()):
    status = {
        'gpl': "GPL, but probably relicensable",
        'nongpl': "License other than GPL",
        'gplonly': "GPL, probably non-relicensable",
    }[cls]
    for path in sorted(paths):
        writer.writerow((status, path))
fd.close()
