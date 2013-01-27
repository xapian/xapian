# Indexes each line of a text file which contains a proper noun.Have assumed that every word beginning with a capital letter is a proper noun.Optionally,One can always use a standard pos tagger such as that  provided by the  nltk library for tagging proper nouns.

# Searching can then be done by specifying the proper noun and the line containing that proper noun will  be displayed.
# Sort of acts like a grep replacement
# Text file is to be specified as a command line arguement

#!/usr/bin/env python
# Copyright (C) 2012 Aarsh Shah
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

import xapian
import string
import sys

if len(sys.argv) < 3:
	# Raise error if path of database or name of file is not specified
	print "Second arguement is the path of the database"
	print "Third arguement is the name of the text file"  
	sys.exit(1)
   

try:
	# Open the database for update, creating a new database if necessary.
	database = xapian.WritableDatabase(sys.argv[1], xapian.DB_CREATE_OR_OPEN)
	indexer = xapian.TermGenerator()
	stemmer = xapian.Stem("english")
	indexer.set_stemmer(stemmer)
	# Open the text file
	main_file=open(sys.argv[2],'r')
	sentences=main_file.readlines()	
	final_sents=[]    # This shall contain all the sentences in the document 
	for sents in sentences:
		a=sents.split('.')
		for each in a:
			final_sents.append(each)        
	# check which sentences contain a proper noun by splitting the sentence into words 	
	for each in final_sents:
		x=each.split(' ')
		for every in x:						
			if every[0]>='A' and every[0]<='Z':
				# The sentence contains a proper noun so index it
				doc = xapian.Document()
				doc.set_data(each)
				indexer.set_document(doc)
				indexer.index_text(every)
				database.add_document(doc)					
				
				
except Exception, e:
    print >> sys.stderr, "Exception: %s" % str(e)
    sys.exit(1)			
		
		
		
	       
	




