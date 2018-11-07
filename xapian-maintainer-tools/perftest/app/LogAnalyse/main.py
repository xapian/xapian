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
"""main.py: Implementation of the LogAnalyseResource.

"""

import os
from WebStack.Generic import ContentType
from SVG import Plot

from tables import *
from templates import *
import config

import plot

def get_run_ids(idlist):
    return [int(i.strip()) for i in idlist.split(',')]

def ids_to_str(runids):
    return u",".join(unicode(runid) for runid in runids)

class LogAnalyseResource:
    """The resource representing the log analysis application."""

    def __init__(self):
        self.db_path = os.path.join(config.db_dir, "logs.db")

    def respond(self, trans):
        path = trans.get_path_without_query("utf-8")
        path = path.split('/')

        if path[1] == u'':
            self.display_index(trans)
        elif path[1] == u'indexrun':
            if len(path) == 2:
                self.display_indexingruns(trans)
            else:
                runids = get_run_ids(path[3])
                if path[2] == u'cmp':
                    self.display_indexingrun_cmp(trans, runids)
                elif path[2] == u'chart':
                    self.display_indexingrun_chart(trans, runids)
                elif path[2] == u'ratechart':
                    if len(path) > 4:
                        self.display_indexingrun_rate_chart(trans, runids, int(path[4]))
                    else:
                        self.display_indexingrun_rate_chart(trans, runids)
                elif path[2] == u'view':
                    self.display_indexingrun(trans, runids, 'view')
                elif path[2] == u'csv':
                    self.display_indexingrun(trans, runids, 'csv')

    def display_index(self, trans):
        trans.set_content_type(ContentType("text/html", "utf-8"))
        def render(t): pass
        out = trans.get_response_stream()
        out.write(render_template(render, "index.html"))

    def display_indexingruns(self, trans):
        trans.set_content_type(ContentType("text/html", "utf-8"))

        database = create_database("sqlite:%s" % self.db_path)
        store = Store(database)

        def render_list(t, item):
            t.test_link.atts['href'] = 'indexrun/view/' + unicode(item.id)
            if item.testname == item.dbname:
                t.test_link.test_name.content = item.testname
            else:
                t.test_link.test_name.content = "%s (%s)" % (item.testname, item.dbname)
            t.backend.content = item.backend
            t.rep_num.content = unicode(item.rep_num)
            t.machine_link.atts['href'] = 'machine/' + unicode(item.machine_id)
            t.machine_link.machine_name.content = unicode(item.machine.name)

        def render(t):
            t.runs.repeat(render_list, store.find(IndexingRun))

        out = trans.get_response_stream()
        out.write(render_template(render, "indexingruns.html"))

    def display_indexingrun(self, trans, runids, fmt):
        runid = runids[0]
        database = create_database("sqlite:%s" % self.db_path)
        store = Store(database)

        def render_list(t, item):
            t.additions.content = unicode(item.adds)
            t.time.content = u"%6f" % item.time

        def render(t):
            items = store.find(IndexingItem,
                               IndexingItem.run_id == runid)
            t.runs.repeat(render_list, items)
            if hasattr(t, 'csv_link'):
                t.csv_link.atts["href"] = "../csv/" + ids_to_str(runids)
            if hasattr(t, 'chart_link'):
                t.chart_link.atts["href"] = "../chart/" + ids_to_str(runids)
            if hasattr(t, 'ratechart_link'):
                t.ratechart_link.atts["href"] = "../ratechart/" + ids_to_str(runids)

        out = trans.get_response_stream()
        if fmt == 'view':
            trans.set_content_type(ContentType("text/html", "utf-8"))
            out.write(render_template(render, "indexingrun.html"))
        elif fmt == 'csv':
            trans.set_content_type(ContentType("text/csv", "utf-8"))
            out.write(render_template(render, "indexingrun.csv"))

    def display_indexingrun_chart(self, trans, runids):
        database = create_database("sqlite:%s" % self.db_path)
        store = Store(database)

        #svg = plot.indexingrun_chart(store, runids)
        #png = plot.svg_to_png(svg)
        #out = trans.get_response_stream()
        #trans.set_content_type(ContentType("image/png"))
        #out.write(png)

        svg = plot.indexingrun_chart(store, runids)
        out = trans.get_response_stream()
        trans.set_content_type(ContentType("image/svg+xml", "utf-8"))
        out.write(svg)

    def display_indexingrun_rate_chart(self, trans, runids, avglen=None):
        database = create_database("sqlite:%s" % self.db_path)
        store = Store(database)
        svg = plot.indexingrun_rate_chart(store, runids, avglen)
        out = trans.get_response_stream()
        trans.set_content_type(ContentType("image/svg+xml", "utf-8"))
        out.write(svg)

    def display_indexingrun_cmp(self, trans, runids, fmt=None):
        database = create_database("sqlite:%s" % self.db_path)
        store = Store(database)

        def render_times(t, time):
            if time is None:
                t.time.content = u""
            else:
                t.time.content = u"%6f" % time

        def render_list(t, item):
            t.additions.content = unicode(item['adds'])
            t.times.repeat(render_times, item['times'])

        def render_heading(t, runid):
            t.content = u"Time (%d)" % runid

        def gen_items():
            items = store.find(IndexingItem,
                               IndexingItem.run_id.is_in(runids)).order_by(IndexingItem.adds)
            result = None
            adds = 0
            for item in items:
                if item.adds != adds:
                    adds = item.adds
                    if result is not None:
                        yield result
                    result = {
                        'adds': adds,
                        'times': [None] * len(runids),
                    }
                result['times'][runids.index(item.run_id)]= item.time

            if result is not None:
                yield result

        def render(t):
            t.timeheadings.repeat(render_heading, runids)

            t.runs.repeat(render_list, gen_items())
            if hasattr(t, 'csv_link'):
                t.csv_link.atts["href"] = "../csv/" + ids_to_str(runids)
            if hasattr(t, 'chart_link'):
                t.chart_link.atts["href"] = "../chart/" + ids_to_str(runids)
            if hasattr(t, 'ratechart_link'):
                t.ratechart_link.atts["href"] = "../ratechart/" + ids_to_str(runids)

        out = trans.get_response_stream()
        if fmt is None:
            trans.set_content_type(ContentType("text/html", "utf-8"))
            out.write(render_template(render, "indexingrun_cmp.html"))
        else:
            trans.set_content_type(ContentType("text/csv", "utf-8"))
            out.write(render_template(render, "indexingrun_cmp.csv"))
