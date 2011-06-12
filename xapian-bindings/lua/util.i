/* lua/util.i: custom lua typemaps for xapian-bindings
 *
 * Copyright (C) 2011 Xiaona Han
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

// "end" is a keyword in Lua, so we rename it to "_end"
%rename("_end") end;

#define XAPIAN_MIXED_VECTOR_QUERY_INPUT_TYPEMAP
/*
 * Check to see what is equivalent to a C++ Vector for the purposes of a Query
 * instantiation.
 * In Lua, we use tables.
 */
%typemap(typecheck, precedence=500) const vector<Xapian::Query> & {
	if (!lua_istable(L, $input)) {
		luaL_typerror(L, $input, "table");
		$1 = 0;
	}
	else {
		$1 = 1;
		int numitems = 0;
		numitems = lua_objlen(L, $input);
		if (numitems == 0) {
			luaL_argerror(L, $input, "table appears to be empty");
			$1 = 0;		
		}
	}
}

/*
 * Convert Lua tables to C++ Vectors for Query instantiation.
 */
%typemap(in) const vector<Xapian::Query> & (vector<Xapian::Query> v) {
	int numitems = 0;
	if (!lua_istable(L, $input)) {
		luaL_typerror(L, $input, "table");
		return 0;
	}
	
	numitems = lua_objlen(L, $input);
	if (numitems == 0) {
		luaL_argerror(L, $input, "table appears to be empty");
		return 0;
	}
	v.reserve(numitems);
	for (int i = 0; i < numitems; ++i) {
		lua_rawgeti(L, $input, i+1);
		if (lua_isstring(L, -1)) {
			size_t len = 0;				
			const char *p = lua_tolstring(L, -1, &len);
			v.push_back(Xapian::Query(string(p, len)));
		}
		else {
			Xapian::Query *subq = 0;
			if(!lua_isuserdata(L, -1) || SWIG_ConvertPtr(L, -1, (void **)&subq, SWIGTYPE_p_Xapian__Query, 0) == -1){
				lua_pop(L, 1);
				luaL_argerror(L, $input, "elements of Tables passed to Query must be either Strings or other Queries");
			}
			if (!subq) {
				SWIG_exception(SWIG_ValueError, "elements of Tables passed to Query must be either Strings or other Queries");
				SWIG_fail;
			}
			v.push_back(*subq);
		}
		lua_pop(L,1);	
	}
   $1 = &v;
}

#define XAPIAN_TERMITERATOR_PAIR_OUTPUT_TYPEMAP
%typemap(out) std::pair<Xapian::TermIterator, Xapian::TermIterator> {
	lua_newtable(L);
	int i = 1;

	for (Xapian::TermIterator iter = $1.first; iter != $1.second; ++iter) {
		lua_pushlstring(L, (*iter).data(), (*iter).length());
		lua_rawseti(L, -2, i++);
	}
	
	SWIG_arg++;
}
