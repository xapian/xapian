/* OmDatabase.java
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
package com.muscat.om;

public class OmDatabase extends OmObject {

    public OmDatabase (OmSettings params) throws OmError { 
	nativePtr = createNativeObject (params);
    }
    protected OmDatabase() {
        super();
    }
    protected native long createNativeObject (OmSettings params);

    protected void finalize () throws Throwable {
	super.finalize ();
    }
    protected native void deleteNativeObject ();

    public native void add_database(OmSettings params);

    public native void add_database(OmDatabase database);

    public native String get_description();
}
