/* omfilereadernode.cc: Implementation of a file reader node
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>
#include "om/omindexernode.h"
#include "om/omerror.h"
#include "node_reg.h"
#include "omdebug.h"
#include <iostream>
#include <fstream>

/** Node which reads a file.
 *
 *  The omfilereader node reads the contents of a file into one
 *  string which can be used by other nodes.
 *
 *  Inputs:
 *  	filename: A string containing the filename to read from.
 *  		  Overridden by the filename parameter if present.
 *
 *  Outputs:
 *  	out: The contents of the file as one string.
 *
 *  Parameters:
 *  	filename: The name of the file.  This is checked before the
 *  		filename input, and takes precedence.
 */
class OmFileReaderNode : public OmIndexerNode {
    public:
	OmFileReaderNode(const OmSettings &config)
		: OmIndexerNode(config),
		  filename(config.get("filename", "")),
		  filename_from_config(filename.length() > 0),
		  have_read_file(false)
	{ }
    private:
	std::string filename;
	bool filename_from_config;
	bool have_read_file;
	void calculate() {
	    std::string fname;
	    if (filename_from_config) {
		fname = filename;
	    } else {
		request_inputs();
		fname = get_input_string("filename");
	    }

	    if (filename_from_config) { 
		if (have_read_file) {
		    // set output to blank
		    set_empty_output("out");
		    return;
		}
	    }
	    std::string output;
	    read_file(output, fname);

	    set_output("out", output);
	    have_read_file = true;
	}

	void config_modified(const std::string &key)
	{
	    if (key == "filename") {
		filename = get_config_string(key);
		filename_from_config = (filename.length() > 0);
	    }
	}

	static void read_file(std::string &s, const std::string &filename)
	{
	    DEBUGLINE(INDEXER, "OmFileReaderNode: reading from " << filename);
	    std::ifstream ifs(filename.c_str());

	    if (!ifs) {
		throw OmInvalidDataError(std::string("Couldn't open file ") +
					 filename);
	    }
	    char buf[1024];

	    do {
		ifs.read(buf, sizeof(buf));
		size_t numbytes = ifs.gcount();
		s.append(buf, numbytes);
		DEBUGLINE(INDEXER, "OmFileReaderNode: appending "
			  << numbytes << " bytes...");
	    } while (!ifs.eof() && ifs.good());
	    
	    if (!ifs.eof()) {
		throw OmInvalidDataError(std::string("Error reading from ")
					 + filename);
	    }
	    DEBUGLINE(INDEXER, "OmFileReaderNode: read " << s.length()
		      << " bytes.");
	}
};

NODE_BEGIN(OmFileReaderNode, omfilereader)
NODE_INPUT("filename", "string", mt_string)
NODE_OUTPUT("out", "string", mt_string)
NODE_END()
