/* quartz_metafile.h: Management of quartz meta-file
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002 Ananova Ltd
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

#ifndef OM_HGUARD_QUARTZ_METAFILE_H
#define OM_HGUARD_QUARTZ_METAFILE_H

#include <string>

/** A class which encapsulates access to the Quartz meta-file, a
 *  small file describing global features of the database.
 */
class QuartzMetaFile {
    public:
	/** Constructor.
	 *
	 * @param filename_	The name of the meta-file
	 */
	QuartzMetaFile(const std::string &filename_);

	/** Destructor */
	~QuartzMetaFile();

	/** Open the meta-file.
	 *
	 *  @except	OmOpeningError if the meta-file was not opened
	 *  		successfully or	is compatible with this version
	 *  		of the library.
	 */
	void open();

	/** Create a new meta-file.
	 *
	 *  @except	OmOpeningError if we couldn't create the meta-file.
	 */
	void create();

	/** Delete the metafile.
	 *
	 *  @except	OmDatabaseError if we couldn't delete the meta-file.
	 */
	void erase();

    private:
	/** The filename of the meta-file */
	std::string filename;
};

#endif /* OM_HGUARD_QUARTZ_METAFILE_H */
