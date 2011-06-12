MODULE = Search::Xapian		PACKAGE = Search::Xapian::MSet

PROTOTYPES: ENABLE

MSet *
new1()
    CODE:
        RETVAL = new MSet();
    OUTPUT:
        RETVAL

MSet *
new2(other)
    MSet *	other
    CODE:
        RETVAL = new MSet(*other);
    OUTPUT:
        RETVAL

void
MSet::fetch1(begin, end)
    MSetIterator *	begin
    MSetIterator *	end
    CODE:
        THIS->fetch(*begin, *end);

void
MSet::fetch2(item)
    MSetIterator *	item
    CODE:
        THIS->fetch(*item);

void
MSet::fetch3()
    CODE:
        THIS->fetch();

percent
MSet::convert_to_percent1(wt)
    weight	wt
    CODE:
        RETVAL = THIS->convert_to_percent(wt);
    OUTPUT:
	RETVAL

percent
MSet::convert_to_percent2(it)
    MSetIterator *	it
    CODE:
        RETVAL = THIS->convert_to_percent(*it);
    OUTPUT:
	RETVAL

doccount
MSet::get_termfreq(tname)
    string	tname
    CODE:
        RETVAL = THIS->get_termfreq(tname);
    OUTPUT:
        RETVAL

weight
MSet::get_termweight(tname)
    string	tname
    CODE:
        RETVAL = THIS->get_termweight(tname);
    OUTPUT:
        RETVAL

doccount
MSet::get_firstitem()

doccount
MSet::get_matches_lower_bound()

doccount
MSet::get_matches_estimated()

doccount
MSet::get_matches_upper_bound()

weight
MSet::get_max_possible()

weight
MSet::get_max_attained()

doccount
MSet::size()
    ALIAS:
	Search::Xapian::MSet::FETCHSIZE = 1

bool
MSet::empty()

void
MSet::swap(other)
        MSet *    other
    CODE:
        THIS->swap(*other);

MSetIterator *
MSet::begin()
    CODE:
        RETVAL = new MSetIterator(THIS->begin());
    OUTPUT:
        RETVAL

MSetIterator *
MSet::end()
    CODE:
        RETVAL = new MSetIterator(THIS->end());
    OUTPUT:
        RETVAL

MSetIterator *
MSet::back()
    CODE:
        RETVAL = new MSetIterator(THIS->back());
    OUTPUT:
        RETVAL

MSetIterator *
MSet::FETCH(doccount i)
# get_msetiterator() alias for backward compatibility.
    ALIAS:
	Search::Xapian::MSet::get_msetiterator = 1
    CODE:
        RETVAL = new MSetIterator((*THIS)[i]);
    OUTPUT:
        RETVAL

string
MSet::get_description()

void
MSet::DESTROY()
