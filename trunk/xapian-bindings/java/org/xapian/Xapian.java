/**
 Copyright (c) 2003, Technology Concepts & Design, Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without modification, are permitted
 provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this list of conditions
 and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright notice, this list of conditions
 and the following disclaimer in the documentation and/or other materials provided with the distribution.

 - Neither the name of Technology Concepts & Design, Inc. nor the names of its contributors may be used to
 endorse or promote products derived from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 THE POSSIBILITY OF SUCH DAMAGE.
 **/
package org.xapian;

import org.xapian.errors.XapianError;

/**
 * Main Entry point for creating Xapian databases.  Provides support for the
 * built-in Xapian namespaces of <code>Auto</code>, <code>InMemory</code>,
 * <code>Quartz</code>, and <code>Remote</code>
 */
public final class Xapian {
    public static final int DB_CREATE_OR_OPEN = 1;
    public static final int DB_CREATE = 2;
    public static final int DB_CREATE_OR_OVERWRITE = 3;
    public static final int DB_OPEN = 4;

    public static final class Auto {
        public static Database open(String path) throws XapianError {
            if (path == null)
                throw new IllegalArgumentException("path cannot be null");
            return new Database(XapianJNI.auto_open(path));
        }

        public static WritableDatabase open(String path, int mode) throws XapianError {
            if (path == null)
                throw new IllegalArgumentException("path cannot be null");
            return new WritableDatabase(XapianJNI.auto_open(path, mode));
        }

        public static Database open_stub(String path) throws XapianError {
            if (path == null)
                throw new IllegalArgumentException("path cannot be null");
            return new Database(XapianJNI.auto_open_stub(path));
        }
    }

    public static final class InMemory {
        public static WritableDatabase open() throws XapianError {
            return new WritableDatabase(XapianJNI.inmemory_open());
        }
    }

    public static final class Quartz {
        public static Database open(String dir) throws XapianError {
            if (dir == null)
                throw new IllegalArgumentException("dir cannot be null");
            return new Database(XapianJNI.quartz_open(dir));
        }

        public static WritableDatabase open(String dir, int action) throws XapianError {
            if (dir == null)
                throw new IllegalArgumentException("dir cannot be null");
            return new WritableDatabase(XapianJNI.quartz_open(dir, action));
        }
    }

    public static final class Remote {
        public static Database open(String program, String args) throws XapianError {
            if (program == null)
                throw new IllegalArgumentException("program cannot be null");
            else if (args == null)
                throw new IllegalArgumentException("args cannot be null");
            return new Database(XapianJNI.remote_open(program, args));
        }

        public static Database open(String program, String args, int timeout) throws XapianError {
            if (program == null)
                throw new IllegalArgumentException("program cannot be null");
            else if (args == null)
                throw new IllegalArgumentException("args cannot be null");
            return new Database(XapianJNI.remote_open(program, args, timeout));
        }

        public static Database open(String host, int port) throws XapianError {
            if (host == null)
                throw new IllegalArgumentException("host cannot be null");
            else if (port < 0)
                throw new IllegalArgumentException("port cannot be negative");
            return new Database(XapianJNI.remote_open(host, port));
        }

        public static Database open(String host, int port, int timeout, int connect_timeout) throws XapianError {
            if (host == null)
                throw new IllegalArgumentException("host cannot be null");
            else if (port < 0)
                throw new IllegalArgumentException("port cannot be negative");
            return new Database(XapianJNI.remote_open(host, port, timeout, connect_timeout));
        }
    }
}
