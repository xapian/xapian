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

import config
import math
import os
import shutil
import subprocess
import tempfile

from SVG import Plot

from tables import *
from templates import *


def indexingrun_chart(store, runids):
    """Return the SVG for a chart of an indexing run.

    """
    plot = Plot.Plot({
        'show_x_guidelines': True,
        'scale_x_integers': True,
        'scale_integers': True,
        'show_y_guidelines': True,
        'show_data_points': False,
        'show_data_values': False,
        'scale_x_integers': True,
        'min_x_value': 0,
        'min_y_value': 0,
        'stagger_x_labels': True,
        'x_title': "Documents",
        #'show_x_title': True, # FIXME - not implemented yet
        'y_title': "Time (s)",
        'show_y_title': True,
        'graph_title': 'Time versus documents',
        'show_graph_title': True,
        'width': 800,
        'height': 600,
        'key': True,
        #'key_position': 'bottom', # FIXME - not implemented yet
    })

    xmax = 0
    for runid in runids:
        items = store.find(IndexingItem,
                           IndexingItem.run_id == runid)
        series = []
        for item in items:
            series.extend((item.adds, item.time))
            xmax = max(xmax, item.adds)
        plot.add_data({'data': series, 'title': 'run %d' % runid})

    xmax_mag = 10.0 ** (int(math.log10(xmax)) - 1)
    xmax = xmax_mag * math.ceil(xmax / xmax_mag)
    plot.scale_x_divisions = xmax / 10.0
    return plot.burn()

def indexingrun_rate_chart(store, runids, avglen=None):
    """Return the SVG for a chart of the rate of an indexing run.

    """
    if avglen is None:
        avglen = 1000
    plot = Plot.Plot({
        'show_x_guidelines': True,
        'scale_x_integers': True,
        'scale_integers': True,
        'show_y_guidelines': True,
        'show_data_points': False,
        'show_data_values': False,
        'scale_x_integers': True,
        'min_x_value': 0,
        'min_y_value': 0,
        'stagger_x_labels': True,
        'x_title': "Documents",
        #'show_x_title': True, # FIXME - not implemented yet
        'y_title': "Time / document (s)",
        'show_y_title': True,
        'graph_title': 'Time per document',
        'show_graph_title': True,
        'graph_subtitle': 'Averaged per %d documents' % avglen,
        'show_graph_subtitle': True,
        'width': 800,
        'height': 600,
        'key': True,
        #'key_position': 'bottom', # FIXME - not implemented yet
    })

    xmax = 0
    for runid in runids:
        items = store.find(IndexingItem,
                           IndexingItem.run_id == runid)
        series = []
        for item in items:
            series.append((item.adds, item.time))

        newseries = []
        last_index = 0
        for adds, time in series:
            if adds % 1000 != 0: continue
            if adds - series[last_index][0] < avglen:
                continue
            while adds - series[last_index + 1][0] >= avglen:
                last_index += 1

            adds_delta = adds - series[last_index][0]
            time_delta = time - series[last_index][1]
            
            newseries.extend((adds, time_delta / adds_delta))
            xmax = max(xmax, adds)
        plot.add_data({'data': newseries, 'title': 'run %d' % runid})

    xmax_mag = 10.0 ** (int(math.log10(xmax)) - 1)
    xmax = xmax_mag * math.ceil(xmax / xmax_mag)
    plot.scale_x_divisions = xmax / 10.0
    return plot.burn()

def svg_to_png(svg):
    tempdir = tempfile.mkdtemp(dir=config.tmp_dir)
    try:
        infile = os.path.join(tempdir, "in.svg")
        outfile = os.path.join(tempdir, "out.png")

        fd = open(infile, "wb")
        fd.write(svg)
        fd.close()

        rc = subprocess.call(["java", "-jar",
                            os.path.join(config.topdir, "app/batik/batik-rasterizer.jar"),
                            "-d", outfile, infile])
        if rc != 0:
            raise "Subprocess error"

        fd = open(outfile, "rb")
        png = fd.read(outfile)
        fd.close()
        return png
    finally:
        shutil.rmtree(tempdir)
