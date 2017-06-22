# Preprocess click data files for obtaining the final clickstream log file.
#
# Copyright (C) 2017 Vivek Pal
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

from ast import literal_eval
from sys import argv

import csv

script_name, search_log, clicks_log = argv

try:
    with open(search_log, 'r') as s:
	with open('log/clickstream.log', 'w+') as f:
	    reader = csv.reader(s, delimiter = '\t')
	    writer = csv.writer(f, delimiter = '\t', quoting = csv.QUOTE_MINIMAL)

	    for row in reader:
		# Skip rows with empty Query string
		if row[1] == '':
		    continue
		# Remove an extra ',' character at the end of the Hitlist string
		hits = row[2]
		hits = hits[:-2]
		row[2] = hits

		# Add Clicks column
		row.append(row[2])

		# Convert Hitlist from string to list
		hits = row[2]
		hits = hits.strip().split(',')
		row[2] = hits

		# Convert Clicks from string to list
		clicks = row[4]
		clicks = clicks.strip().split(',')
		row[4] = clicks

		# Initialise Clicks list elements to zero
		for i in range(len(row[4])):
		    row[4][i] = str(0)

		writer.writerow(row)
except IOError:
	print "Could not read files."

try:
    with open('log/clickstream.log', 'r+') as s:
	with open(clicks_log, 'r') as f:
	    with open('log/final.log', 'w+') as w:
		reader1 = csv.reader(s, delimiter = '\t')

		# Store the contents of clicks_log in a list object
		reader2 = list(csv.reader(f, delimiter = '\t'))

		writer = csv.writer(w, delimiter = '\t', quoting = csv.QUOTE_MINIMAL)

		# Add headers to final log file
		writer.writerow(["QueryID", "Query", "Hits", "Offset", "Clicks"])

		for row1 in reader1:
		    # Convert column 3 and column 5 values to list type
		    l2 = literal_eval(row1[2])
		    l4 = literal_eval(row1[4])

		    for row2 in reader2:
			docid = ""
			# Find matching query IDs in clickstream.log and clicks_log
			if row1[0] == row2[0]:
			    docid = row2[1]
			
			index = -1
			# Find the index of clicked doc ID in "Hits" list
			for i in range(len(l2)):
			    if docid == l2[i]:
				index = i
			# Update the count representing the number of clicks made
			# on clicked document from "Hits" list
			for j in range(len(l4)):
			    if j == index:
				count = int(l4[index])
				count = count + 1
				l4[j] = str(count)
		    # Update "Clicks" list with updated values
		    row1[4] = l4
		    # Write the contents of row iterator to the final log file
		    writer.writerow(row1)
except IOError:
	print "Could not read files."