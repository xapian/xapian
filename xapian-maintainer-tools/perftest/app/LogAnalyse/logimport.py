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
"""logimport.py: Import uploaded logs into the system.

"""

import os
import shutil
import tempfile
import time
import threading

from parselog import *

from tables import *
from storm.exceptions import OperationalError

import config

__all__ = ("LogImporter", )

class LogImporter(threading.Thread):
    """A thread which watches an incoming directory and imports logs from it.

    """

    def __init__(self):
        """Initialse the log importer.

        """
        self.queue_dir = config.queue_dir
        self.archive_dir = config.archive_dir
        self.db_dir = config.db_dir
        self.db_path = os.path.join(config.db_dir, "logs.db")
        self.log_path = config.log_path
        self.stop_now = threading.Event()
        self.started = threading.Event()

        threading.Thread.__init__(self)

    def wait_for_start(self):
        """Block until the thread has started.

        When this returns, the database is

        """
        self.started.wait()

    def stop(self):
        """Stop the importing thread.

        """
        self.stop_now.set()

    def run(self):
        """Main loop for the log importing thread.

        """
        try:
            self.database = create_database("sqlite:%s" % self.db_path)
            self.store = Store(self.database)
            # Create the tables if they're not already there
            try:
                version = self.store.find(MetaInfo, MetaInfo.key == u"version").one()
                version = int(version.value)
            except OperationalError, e:
                version = 0
            if version != schema_version:
                print "Creating new database version"
                self.store.close()
                del self.database
                os.unlink(self.db_path)
                self.database = create_database("sqlite:%s" % self.db_path)
                self.store = Store(self.database)
                create_tables(self.store)
        finally:
            self.started.set()

        while True:
            if self.stop_now.isSet():
                break
            self.check_imports()
            self.stop_now.wait(1.0)

    def log(self, message):
        fd = open(self.log_path, "ab")
        fd.write("%s: %s" % (int(time.time()), message))
        fd.close()

    def check_imports(self):
        """Check the incoming directory for imports.

        """
        for dirpath, dirnames, filenames in os.walk(self.queue_dir):
            for filename in filenames:
                if filename.endswith('.xml'):
                    path = os.path.join(dirpath, filename)
                    self.import_file(path)

    def archive_file(self, path):
        #basename = os.path.splitext(os.path.basename(path))[0]
        basename = 'perflog'
        fd, fpath = tempfile.mkstemp('.xml', basename, self.archive_dir)
        shutil.move(path, fpath)

    def import_file(self, path):
        try:
            parsed = parselog(path)
        except LogParseError, e:
            self.log("Couldn't import %r:%s\n" % (os.path.basename(path), e))
            self.archive_file(path)
            return
        self.log("Parsed %r\n" % (os.path.basename(path), ))
        self.store_parsed_log(parsed)
        self.archive_file(path)

    def store_parsed_log(self, parsed):
        machine = Machine()
        machine.physmem = parsed.machineinfo.physmem
        self.store.add(machine)

        for infos in parsed.tests.itervalues():
            for info in infos:
                for indexrun in info.indexruns:
                    run = IndexingRun()
                    run.machine = machine
                    run.testname = info.name
                    run.dbname = indexrun.dbname
                    run.backend = info.backend
                    run.rep_num = info.rep_num
                    self.store.add(run)

                    for runitem in indexrun.items:
                        item = IndexingItem()
                        item.run = run
                        item.adds = runitem[1]
                        item.time = runitem[0]
                        self.store.add(item)

        self.store.commit()


if __name__ == '__main__':
    import sys, time
    if len(sys.argv) != 5:
        print "Usage: parselog.py"
        sys.exit(1)
    importer = LogImporter()
    importer.start()
    try:
        while True:
            time.sleep(100000)
    except:
        importer.stop()
        importer.join()
        raise
