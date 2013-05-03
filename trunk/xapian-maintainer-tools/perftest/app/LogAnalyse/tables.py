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
"""tables.py: Tables used to hold the logs.

"""

from storm.locals import *

schema_version = 1

class MetaInfo(object):
    __storm_table__ = "metainfo"
    key = Unicode(primary=True)
    value = Unicode()
    def __init__(self, key, value):
        self.key = key
        self.value = value

class Machine(object):
    __storm_table__ = "machine"
    id = Int(primary=True)
    name = Unicode()
    physmem = Unicode()

class IndexingRun(object):
    __storm_table__ = "indexingrun"
    id = Int(primary = True)
    machine_id = Int()
    machine = Reference(machine_id, Machine.id)
    testname = Unicode()
    dbname = Unicode()
    backend = Unicode()
    rep_num = Int()

class IndexingItem(object):
    __storm_table__ = "indexingitem"
    __storm_primary__ = "run_id", "adds"
    run_id = Int()
    run = Reference(run_id, IndexingRun.id)
    adds = Int()
    time = Float()

IndexingRun.items = ReferenceSet(IndexingRun.id, IndexingItem.run_id)

def create_tables(store):
    store.execute("""CREATE TABLE metainfo
    (key VARCHAR PRIMARY KEY,
     value VARCHAR)
    """)
    store.add(MetaInfo(u"version", unicode(schema_version)))
    store.execute("""CREATE TABLE machine
    (id INTEGER PRIMARY KEY,
     name VARCHAR,
     physmem VARCHAR)
    """)
    store.execute("""CREATE TABLE indexingrun
    (id INTEGER PRIMARY KEY,
     machine_id INTEGER,
     testname VARCHAR,
     dbname VARCHAR,
     backend VARCHAR,
     rep_num INTEGER)
    """)
    store.execute("""CREATE TABLE indexingitem
    (run_id INTEGER NOT NULL,
     adds INTEGER NOT NULL,
     time FLOAT,
     PRIMARY KEY (run_id, adds))
    """)
    store.commit()
