%{
/* fake_dbfactory.i: Fake classes for xapian/dbfactory.h functions.
 *
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002,2003,2005 James Aylett
 * Copyright 2002,2003,2004,2005,2006,2007,2008,2009,2010,2011 Olly Betts
 * Copyright 2007 Lemur Consulting Ltd
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
%}

namespace Xapian {

/* Lie to SWIG that Auto, etc are classes with static methods rather than
   namespaces so it wraps it as we want in C# and Java. */
class Auto {
  private:
    Auto();
    ~Auto();
  public:
    static
    Database open_stub(const string & file);
};

class InMemory {
  private:
    InMemory();
    ~InMemory();
  public:
    static
    WritableDatabase open();
};

class Remote {
  private:
    Remote();
    ~Remote();
  public:
    static
    Database open(const std::string &host, unsigned int port, unsigned timeout, unsigned connect_timeout);
    static
    Database open(const std::string &host, unsigned int port, unsigned timeout = 10000);

    static
    WritableDatabase open_writable(const std::string &host, unsigned int port, unsigned timeout, unsigned connect_timeout);
    static
    WritableDatabase open_writable(const std::string &host, unsigned int port, unsigned timeout = 10000);

    static
    Database open(const std::string &program, const std::string &args, unsigned timeout = 10000);

    static
    WritableDatabase open_writable(const std::string &program, const std::string &args, unsigned timeout = 10000);
};

}
