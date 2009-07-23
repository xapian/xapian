#!/usr/bin/env python

import math
import re
import sys
import wx

class FileAccesses(object):
    def __init__(self):
        self.max_offset = 0
        self.data = []

    def append(self, start_time, offset, elapsed, optype):
        self.data.append((start_time, offset, elapsed, optype))
        if self.max_offset < offset:
            self.max_offset = offset

    def __iter__(self):
        return iter(self.data)

class AllAccesses(object):
    def __init__(self):
        self.data = {}
        self.total_time = 0

    def append(self, filename, offset, elapsed, optype):
        fileaccesses = self.data.get(filename, None)
        if fileaccesses is None:
            fileaccesses = FileAccesses()
            self.data[filename] = fileaccesses
        fileaccesses.append(self.total_time, offset, elapsed, optype)
        self.total_time += elapsed

    def files_count(self):
        return len(self.data)

    def sum_max_offset(self):
        return sum(x.max_offset for x in self.data.itervalues())

    def __iter__(self):
        keys = self.data.keys()
        keys.sort()
        for key in keys:
            yield key, self.data[key]

    def __len__(self):
        return len(self.data)

class DrawPanel(wx.Panel):
    """draw a line on a panel's wx.PaintDC surface/canvas"""
    def __init__(self, parent, data):
        wx.Panel.__init__(self, parent, -1)
        # bind the panel to the paint event
        wx.EVT_PAINT(self, self.onPaint)

        self.data = AllAccesses()
        for filename, offset, elapsed, optype in data:
            #elapsed = 0.0005
            self.data.append(filename, offset, elapsed, optype)

    def onPaint(self, event=None):
        # this is the wx drawing surface/canvas
        dc = wx.PaintDC(self)
        dc.Clear()

        wid, hgt = self.GetClientSizeTuple()
        left = 10
        bot = 10
        top = 10
        right = 10

        titlehgt = 20

        xscale = float(wid - left - right) / self.data.total_time
        yscale = float(hgt - top - bot - titlehgt * len(self.data)) / self.data.sum_max_offset()

        pointwid = 1
        print "Drawing"
        dc.SetPen(wx.Pen("grey", 1))
        dc.SetBrush(wx.Brush("#eeeeee"))
        dc.DrawRectangle(0, 0, wid, hgt)

        counts = [0, 0, 0]
        maxoffset_sum = 0
        for num, (filename, data) in enumerate(self.data):
            base_y = int(math.floor(num * titlehgt + (bot + yscale * maxoffset_sum)))
            top_y = int(math.ceil(base_y + (yscale * data.max_offset)))

            dc.SetPen(wx.Pen("grey", 1))
            dc.SetBrush(wx.Brush("white"))
            dc.DrawRectangle(left - 1, hgt - (top_y + 1), wid + 2 - right - left, top_y - base_y + 2)
            twid, thgt = dc.GetTextExtent(filename)
            dc.DrawText(filename, 0, hgt - (top_y + thgt))

            maxoffset_sum += data.max_offset

            for starttime, offset, elapsed, optype in data:
                xpos = left + xscale * starttime
                pointwid = xscale * elapsed
                if pointwid < 1: pointwid = 1

                if optype == 'w':
                    dc.SetPen(wx.Pen("blue", 1))
                    counts[2] += 1
                    y = base_y + (yscale * offset)
                    dc.DrawLine(xpos, hgt - y, xpos + pointwid, hgt - y)

                elif optype == 'r':
                    if elapsed < 0.001:
                        dc.SetPen(wx.Pen("dark green", 1))
                        counts[0] += 1
                    else:
                        dc.SetPen(wx.Pen("red", 1))
                        counts[1] += 1
                    y = base_y + (yscale * offset)
                    dc.DrawLine(xpos, hgt - y, xpos + pointwid, hgt - y)

                elif optype == 's':
                    dc.SetPen(wx.Pen("light blue", 1))
                    dc.DrawLine(xpos, hgt - base_y, xpos, hgt - top_y)

        print "Drawn: [green, red, blue] =", counts
        if counts[1] > 0:
            print "blue/red =", float(counts[0]) / counts[1]
        print "Total time: %f" % (self.data.total_time,)
        print "Avg time: %f" % (self.data.total_time / (counts[0] + counts[1] + counts[2]))

def show_data(app, data, title):
    frame = wx.Frame(None, -1, title, size=(1500, 1000))
    dp = DrawPanel(frame, data)
    frame.Show(True)

def read_data(filename):
    fd = open(filename)
    fds = {}
    try:
        data = []
        just_seeked = None
        for line in fd:
            line = line.strip()
            if line[0] == 'o':
                fdnum, filename = line[1:].split(',', 1)
                fds[int(fdnum)] = filename
                continue

            if line[0] == 'r':
                fdnum, offset, len = map(int, line[1:].split(',', 2))
                filename = fds.get(fdnum, str(fdnum))
                data.append((filename, offset, len, line[0]))
                continue

            if line[0] == 'w':
                fdnum, offset, len = map(int, line[1:].split(',', 2))
                filename = fds.get(fdnum, str(fdnum))
                data.append((filename, offset, len, line[0]))
                continue

            if line[0] == 's':
                fdnum = int(line[1:-1])
                data.append((filename, 0, 0, line[0]))
                continue

        return data
    finally:
        fd.close()

if __name__ == '__main__':
    app = wx.App()
    # data file should be produced by strace -T
    filename = sys.argv[1]
    data = read_data(filename)
    show_data(app, data, "%d io calls, from %s" % (len(data), filename))
    app.MainLoop()
