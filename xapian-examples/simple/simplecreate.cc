/* simplecreate.cc: Simplest possible database creation
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include <om/om.h>
#include <sys/stat.h> // For mkdir

int main(int argc, char *argv[])
{
    // Simplest possible options parsing: we just require one parameter.
    if(argc != 2) {
	std::cout << "usage: " << argv[0] <<
		" <path to database>" <<
		std::endl;
	exit(1);
    }
    
    // Catch any OmError exceptions thrown
    try {
	// Create the directory for the database
	mkdir(argv[1], 0755);

	// Create the database
	OmSettings settings;
	settings.set("backend", "quartz");
	settings.set("database_create", true);
	settings.set("quartz_dir", argv[1]);
	OmWritableDatabase database(settings);
    }
    catch(OmError &error) {
	std::cout << "Exception: "  << error.get_msg() << std::endl;
    }
}
