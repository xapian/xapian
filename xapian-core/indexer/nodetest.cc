/* nodetest.cc: temporary program for trying out OmIndexerNode
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

#include "omindexernode.h"
#include "testnodes.h"

int main() {
    Message origmsg(new BasicMessage());
    origmsg->name = "foo";
    origmsg->value = "bar";
    OmOrigNode orig(origmsg);

    SplitNode split;
    split.connect_input("in", &orig, "out");

    ReverseNode reverse;
    reverse.connect_input("in", &split, "out1");

    ConcatNode concat;
    concat.connect_input("in1", &reverse, "out");
    concat.connect_input("in2", &split, "out2");

    Message result = concat.get_output("out");

    cout << "Name: " << result->name << endl;
    cout << "Value: " << result->value << endl;
}
