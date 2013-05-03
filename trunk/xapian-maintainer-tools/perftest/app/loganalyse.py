#!/usr/bin/env python
#
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
"""Start the LogAnalyse webapp, using twisted as the server.

"""

from WebStack.Adapters.Twisted import deploy
from LogAnalyse import config, LogAnalyseResource, LogImporter

importer = LogImporter()
importer.start()
importer.wait_for_start()
try:
    deploy(LogAnalyseResource(),
           handle_errors=0, address=('xapian.org', 8020))
finally:
    importer.stop()
    importer.join()
