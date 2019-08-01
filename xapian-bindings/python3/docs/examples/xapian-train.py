#!/usr/bin/env python
#
# Simple example script demonstrating how to train and save the model.
#
# Copyright (C) 2019 Vaibhav Kansagara
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

import sys
import xapian
import xapianletor

# We require two command line arguments.
if len(sys.argv) != 3:
    print("Usage: %s DATABASE TRAINING_FILE" % sys.argv[0], file=sys.stderr)
    sys.exit(1)

try:
    db_path = sys.argv[1]
    trainingfile = sys.argv[2]

    # Initialise Ranker object.
    # See Ranker documentation for available Ranker subclass options.
    ranker = xapianletor.ListNETRanker()

    # Set database
    ranker.set_database_path(db_path)

    # Train the model
    ranker.train_model(trainingfile)

except Exception as e:
    print("Exception: %s" % str(e), file=sys.stderr)
    sys.exit(1)
