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

%define SUB_CLASS(NS, CLASS)
%{
class lua##CLASS : public NS::CLASS {
	int r;
	lua_State* L;

	public:
		lua##CLASS(lua_State* S) {
			L = S;
			if (!lua_isfunction(L, -1)) {
				luaL_typerror(L, -1, "function");
			}
			r = luaL_ref(L, LUA_REGISTRYINDEX);
		}

		~lua##CLASS() {
			luaL_unref(L, LUA_REGISTRYINDEX, r);
		}

		bool operator()(const std::string &term) const {
			lua_rawgeti(L, LUA_REGISTRYINDEX, r);
			if (!lua_isfunction(L, -1)) {
				luaL_typerror(L, -1, "function");
			}

			lua_pushstring(L, (char *)term.c_str());
			if (lua_pcall(L, 1, 1, 0) != 0){
				luaL_error(L, "error running function: %s", lua_tostring(L, -1));
			}
			if (!lua_isboolean(L, -1)) {
				luaL_error(L, "function must return a boolean");
			}
			bool result = lua_toboolean(L, -1);
			lua_pop(L, 1);
			return result;
		}
};
%}

%enddef

SUB_CLASS(Xapian, ExpandDecider)
SUB_CLASS(Xapian, Stopper)

%{
class luaMatchDecider : public Xapian::MatchDecider {
	int r;
	lua_State* L;

	public:
		luaMatchDecider(lua_State* S) {
			L = S;
			if (!lua_isfunction(L, -1)) {
				luaL_typerror(L, -1, "function");
			}
			r = luaL_ref(L, LUA_REGISTRYINDEX);
		}

		~luaMatchDecider() {
			luaL_unref(L, LUA_REGISTRYINDEX, r);
		}

		bool operator()(const Xapian::Document &doc) const {
			lua_rawgeti(L, LUA_REGISTRYINDEX, r);
			if (!lua_isfunction(L, -1)) {
				luaL_typerror(L, -1, "function");
			}

			SWIG_NewPointerObj(L, &doc, SWIGTYPE_p_Xapian__Document, 0);
			if (lua_pcall(L, 1, 1, 0) != 0){
				luaL_error(L, "error running function: %s", lua_tostring(L, -1));
			}
			if (!lua_isboolean(L, -1)) {
				luaL_error(L, "function must return a boolean");
			}
			bool result = lua_toboolean(L, -1);
			lua_pop(L, 1);
			return result;
		}
};
%}

%{
class luaStemImplementation : public Xapian::StemImplementation {
	int r;
	lua_State* L;

	public:
		luaStemImplementation(lua_State* S) {
			L = S;
			if (!lua_isfunction(L, -1)) {
				luaL_typerror(L, -1, "function");
			}
			r = luaL_ref(L, LUA_REGISTRYINDEX);
		}

		~luaStemImplementation() {
			luaL_unref(L, LUA_REGISTRYINDEX, r);
		}

		std::string operator()(const std::string &word) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, r);
			if (!lua_isfunction(L, -1)) {
				luaL_typerror(L, -1, "function");
			}

			SWIG_NewPointerObj(L, &word, SWIGTYPE_p_std__string, 0);
			if (lua_pcall(L, 1, 1, 0) != 0){
				luaL_error(L, "error running function: %s", lua_tostring(L, -1));
			}
			if (!lua_isstring(L, -1)) {
				luaL_error(L, "function must return a string");
			}
			std::string result(lua_tostring(L, -1));
			lua_pop(L, 1);
			return result;
		}
		std::string get_description() const {
			lua_rawgeti(L, LUA_REGISTRYINDEX, r);
			if (!lua_isfunction(L, -1)) {
				luaL_typerror(L, -1, "function");
			}

			if (lua_pcall(L, 0, 1, 0) != 0){
				luaL_error(L, "error running function: %s", lua_tostring(L, -1));
			}
			if (!lua_isstring(L, -1)) {
				luaL_error(L, "function must return a string");
			}
			
			std::string result(lua_tostring(L, -1));
			lua_pop(L, 1);
			return result;
		}
};
%}

%{
class luaKeyMaker : public Xapian::KeyMaker {
	int r;
	lua_State* L;

	public:
		luaKeyMaker(lua_State* S) {
			L = S;
			if (!lua_isfunction(L, -1)) {
				luaL_typerror(L, -1, "function");
			}
			r = luaL_ref(L, LUA_REGISTRYINDEX);
		}

		~luaKeyMaker() {
			luaL_unref(L, LUA_REGISTRYINDEX, r);
		}

		std::string operator()(const Xapian::Document &doc) const {
			lua_rawgeti(L, LUA_REGISTRYINDEX, r);
			if (!lua_isfunction(L, -1)) {
				luaL_typerror(L, -1, "function");
			}

			SWIG_NewPointerObj(L, &doc, SWIGTYPE_p_Xapian__Document, 0);
			if (lua_pcall(L, 1, 1, 0) != 0){
				luaL_error(L, "error running function: %s", lua_tostring(L, -1));
			}
			if (!lua_isstring(L, -1)) {
				luaL_error(L, "function must return a string");
			}
			std::string result(lua_tostring(L, -1));
			lua_pop(L, 1);
			return result;
		}
};
%}

%{
class luaValueRangeProcessor : public Xapian::ValueRangeProcessor {
	int r;
	lua_State* L;

	public:
		luaValueRangeProcessor(lua_State* S) {
			L = S;
			if (!lua_isfunction(L, -1)) {
				luaL_typerror(L, -1, "function");
			}
			r = luaL_ref(L, LUA_REGISTRYINDEX);
		}

		~luaValueRangeProcessor() {
			luaL_unref(L, LUA_REGISTRYINDEX, r);
		}

		Xapian::valueno operator()(std::string &begin, std::string &end) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, r);
			if (!lua_isfunction(L, -1)) {
				luaL_typerror(L, -1, "function");
			}

			SWIG_NewPointerObj(L, &begin, SWIGTYPE_p_std__string, 0);
			SWIG_NewPointerObj(L, &end, SWIGTYPE_p_std__string, 0);

			if (lua_pcall(L, 2, 1, 0) != 0){
				luaL_error(L, "error running function: %s", lua_tostring(L, -1));
			}
			if (!lua_isnumber(L, -1)) {
				luaL_error(L, "function must return a nubmer");
			}
			Xapian::valueno result(lua_tonumber(L, -1));
			lua_pop(L, 1);
			return result;
		}
};
%}

%{
class luaMatchSpy : public Xapian::MatchSpy {
	int r;
	lua_State* L;

	public:
		luaMatchSpy(lua_State* S) {
			L = S;
			if (!lua_isfunction(L, -1)) {
				luaL_typerror(L, -1, "function");
			}
			r = luaL_ref(L, LUA_REGISTRYINDEX);
		}

		~luaMatchSpy() {
			luaL_unref(L, LUA_REGISTRYINDEX, r);
		}

		void operator()(const Xapian::Document &doc, Xapian::weight wt) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, r);
			if (!lua_isfunction(L, -1)) {
				luaL_typerror(L, -1, "function");
			}

			SWIG_NewPointerObj(L, &doc, SWIGTYPE_p_Xapian__Document, 0);
			SWIG_NewPointerObj(L, &wt, SWIGTYPE_p_Xapian__Weight, 0);
			if (lua_pcall(L, 2, 1, 0) != 0){
				luaL_error(L, "error running function: %s", lua_tostring(L, -1));
			}
			return;
		}
};
%}

%define SUB_CLASS_TYPEMAPS(NS, CLASS)

%typemap(typecheck, precedence=100) NS::CLASS * {
	void *ptr;
	if (lua_isfunction(L, $input) || (SWIG_isptrtype(L, $input) && !SWIG_ConvertPtr(L, $input, (void **) &ptr, $descriptor(CLASS), 0))) {
		$1 = 1;
	}
	else {
		$1 = 0;
	}
}
%typemap(in) NS::CLASS * {
	if (lua_isfunction(L, $input)) {
		$1 = new lua##CLASS(L);
	}
	else {
		if (!SWIG_IsOK(SWIG_ConvertPtr(L, $input, (void**)&$1, $descriptor(CLASS), 0))){
			SWIG_fail;
		}
	}
}

%enddef
SUB_CLASS_TYPEMAPS(Xapian, MatchDecider)
SUB_CLASS_TYPEMAPS(Xapian, ExpandDecider)
SUB_CLASS_TYPEMAPS(Xapian, Stopper)
SUB_CLASS_TYPEMAPS(Xapian, StemImplementation)
SUB_CLASS_TYPEMAPS(Xapian, KeyMaker)
SUB_CLASS_TYPEMAPS(Xapian, ValueRangeProcessor)

%luacode {
function xapian.Iterator(begin, _end)
	local iter = begin;
	local isFirst = 1 
	return function()
		if iter:equals(_end) then
			return nil
		else
			if isFirst == 1 then
				isFirst = 0;
			return iter
			else
				iter:next()
				if iter:equals(_end) then
					return nil
				end
			return iter
			end
		end
	end
end
}

#define XAPIAN_MIXED_SUBQUERIES_BY_ITERATOR_TYPEMAP

%typemap(typecheck, precedence=500) (XapianSWIGQueryItor qbegin, XapianSWIGQueryItor qend) {
    $1 = lua_istable(L, $input);
    /* FIXME: if we add more array typemaps, we'll need to check the elements
     * of the array here to disambiguate. */
}

%{
class XapianSWIGQueryItor {
	lua_State* L;
	int index;
	int i;

  public:
    XapianSWIGQueryItor() { }

    XapianSWIGQueryItor(lua_State * S, int index_, int n)
	: L(S), index(index_), i(n) { }

    XapianSWIGQueryItor & operator++() {
			++i;
			return *this;
    }

    Xapian::Query operator*() const {
			lua_rawgeti(L, index, i+1);
			if (lua_isstring(L, -1)) {
				size_t len = 0;
				const char *p = lua_tolstring(L, -1, &len);
				lua_pop(L,1);
				return Xapian::Query(string(p, len));
			}
			else {
				Xapian::Query *subq = 0;
				if(!lua_isuserdata(L, -1) || SWIG_ConvertPtr(L, -1, (void **)&subq, SWIGTYPE_p_Xapian__Query, 0) == -1){
					lua_pop(L, 1);
					luaL_argerror(L, index, "elements of Tables passed to Query must be either Strings or other Queries");
				}
	
				lua_pop(L, 1);
				return *subq;
			}
    }

    bool operator==(const XapianSWIGQueryItor & o) {
			return i == o.i;
    }

    bool operator!=(const XapianSWIGQueryItor & o) {
				return !(*this == o);
    }

    typedef std::input_iterator_tag iterator_category;
    typedef Xapian::Query value_type;
    typedef Xapian::termcount_diff difference_type;
    typedef Xapian::Query * pointer;
    typedef Xapian::Query & reference;
};

%}

%typemap(in) (XapianSWIGQueryItor qbegin, XapianSWIGQueryItor qend) {
	if (lua_istable(L, $input)) {
		$1 = XapianSWIGQueryItor(L, $input, 0);
		$2 = XapianSWIGQueryItor(L, $input, lua_objlen(L, $input));
	}
	else {
		$1 = $2 = XapianSWIGQueryItor(L, $input, 0);
  }
}

%define OUTPUT_ITERATOR_METHODS(NS, CLASS, ITERATOR_CLASS, ITERATOR_BEGIN, ITERATOR_END, DEREF_METHOD, PARAMETER_NAME, PARAMETER_VALUE)

%extend NS::CLASS {
	std::pair<NS::ITERATOR_CLASS , NS::ITERATOR_CLASS> DEREF_METHOD(PARAMETER_NAME) {
		return std::make_pair($self->ITERATOR_BEGIN(PARAMETER_VALUE), $self->ITERATOR_END(PARAMETER_VALUE));
	}
}

%typemap(out) std::pair<NS::ITERATOR_CLASS, NS::ITERATOR_CLASS> {
	lua_getglobal(L, "xapian");
	lua_pushstring(L, "Iterator");
	lua_gettable(L, -2);
	lua_remove(L, -2);

	if (!lua_isfunction(L, -1)) {
				luaL_typerror(L, -1, "function");
	}

	NS::ITERATOR_CLASS * begin = new NS::ITERATOR_CLASS((const NS::ITERATOR_CLASS &)$1.first);
	SWIG_NewPointerObj(L, (void *) begin, SWIGTYPE_p_##NS##__##ITERATOR_CLASS, 1);
	
	NS::ITERATOR_CLASS * end = new NS::ITERATOR_CLASS((const NS::ITERATOR_CLASS &)$1.second);
	SWIG_NewPointerObj(L, (void *) end, SWIGTYPE_p_##NS##__##ITERATOR_CLASS, 1);

	if (lua_pcall(L, 2, 1, 0) != 0) {
		luaL_error(L, "error running function: %s", lua_tostring(L, -1));
	}

	SWIG_arg++;
}

%enddef

OUTPUT_ITERATOR_METHODS(Xapian, Query, TermIterator, get_terms_begin, get_terms_end, get_terms, void, )

OUTPUT_ITERATOR_METHODS(Xapian, ESet, ESetIterator, begin, end, terms, void, )

OUTPUT_ITERATOR_METHODS(Xapian, MSet, MSetIterator, begin, end, items, void, )

OUTPUT_ITERATOR_METHODS(Xapian, Document, TermIterator, termlist_begin, termlist_end, termlist, void, )
OUTPUT_ITERATOR_METHODS(Xapian, Document, ValueIterator, values_begin, values_end, values, void, )

OUTPUT_ITERATOR_METHODS(Xapian, Enquire, TermIterator, get_matching_terms_begin, get_matching_terms_end, get_matching_terms, Xapian::docid did, did)
OUTPUT_ITERATOR_METHODS(Xapian, Enquire, TermIterator, get_matching_terms_begin, get_matching_terms_end, get_matching_terms, const MSetIterator &it, it)

OUTPUT_ITERATOR_METHODS(Xapian, ValueCountMatchSpy, TermIterator, values_begin, values_end, values, void, )
OUTPUT_ITERATOR_METHODS(Xapian, ValueCountMatchSpy, TermIterator, top_values_begin, top_values_end, top_values, size_t maxvalues, maxvalues)

OUTPUT_ITERATOR_METHODS(Xapian, Database, TermIterator, allterms_begin, allterms_end, allterms, void, )
OUTPUT_ITERATOR_METHODS(Xapian, Database, TermIterator, spellings_begin, spellings_end, spellings, void, )
OUTPUT_ITERATOR_METHODS(Xapian, Database, PostingIterator, postlist_begin, postlist_end, postlist, const std::string &tname, tname)
OUTPUT_ITERATOR_METHODS(Xapian, Database, TermIterator, termlist_begin, termlist_end, termlist, Xapian::docid did, did)
OUTPUT_ITERATOR_METHODS(Xapian, Database, ValueIterator, valuestream_begin, valuestream_end, valuestream, Xapian::valueno slot, slot)
OUTPUT_ITERATOR_METHODS(Xapian, Database, TermIterator, allterms_begin, allterms_end, allterms, const std::string &prefix, prefix)
OUTPUT_ITERATOR_METHODS(Xapian, Database, TermIterator, synonyms_begin, synonyms_end, synonyms, const std::string &term, term)
OUTPUT_ITERATOR_METHODS(Xapian, Database, TermIterator, synonym_keys_begin, synonym_keys_end, synonym_keys, const std::string &prefix, prefix)
OUTPUT_ITERATOR_METHODS(Xapian, Database, TermIterator, metadata_keys_begin, metadata_keys_end, metadata_keys, const std::string &prefix, prefix)

%extend Xapian::Database {
	std::pair<Xapian::PositionIterator , Xapian::PositionIterator> positionlist(Xapian::docid did, const std::string &tname) {
		return std::make_pair($self->positionlist_begin(did, tname), $self->positionlist_end(did, tname));
	}
}

%typemap(out) std::pair<Xapian::PositionIterator, Xapian::PositionIterator> {
	lua_getglobal(L, "xapian");
	lua_pushstring(L, "Iterator");
	lua_gettable(L, -2);
	lua_remove(L, -2);

	if (!lua_isfunction(L, -1)) {
				luaL_typerror(L, -1, "function");
	}

	Xapian::PositionIterator * begin = new Xapian::PositionIterator((const Xapian::PositionIterator &)$1.first);
	SWIG_NewPointerObj(L, (void *) begin, SWIGTYPE_p_Xapian__PositionIterator, 1);
	
	Xapian::PositionIterator * end = new Xapian::PositionIterator((const Xapian::PositionIterator &)$1.second);
	SWIG_NewPointerObj(L, (void *) end, SWIGTYPE_p_Xapian__PositionIterator, 1);

	if (lua_pcall(L, 2, 1, 0) != 0) {
		luaL_error(L, "error running function: %s", lua_tostring(L, -1));
	}

	SWIG_arg++;
}


