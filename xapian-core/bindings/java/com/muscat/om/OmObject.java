/* OmObject.java: This class is the root of the OM classes which
 *                can be instantiated from Java.
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

import java.util.*;

/**
 * This class is the root of the OM classes which can be instantiated from Java.
 * When the Java object is created, a corresponding native object is also instantiated.
 */

public abstract class OmObject {
    protected long nativePtr;             // pointer to the C++ object

    /**
     * when the Java object is garbage collected, delete the native object
     */
    protected void finalize () throws Throwable {
	if (nativePtr != 0) {
	    //_decr_object_count (this.getClass().getName());
	    deleteNativeObject ();
	    nativePtr = 0;
	}
    }
    
    protected abstract void deleteNativeObject ();

    static {
	System.loadLibrary ("omusjava");
    }
}
