MODULE = Search::Xapian		PACKAGE = Search::Xapian::MSet

PROTOTYPES: ENABLE

OmMSet *
new1();
    CODE:
        RETVAL = new OmMSet();
    OUTPUT:
        RETVAL

OmMSet *
new2(other);
    OmMSet * other
    CODE:
        RETVAL = new OmMSet(* other);
    OUTPUT:
        RETVAL
 
void
OmMSet::fetch1(begin, end)
    OmMSetIterator *    begin
    OmMSetIterator *    end
    CODE:
        THIS->fetch(* begin, * end);

void
OmMSet::fetch2(item)
    OmMSetIterator *    item
    CODE:
        THIS->fetch(* item);

void
OmMSet::fetch3()
    CODE:
        THIS->fetch();

void
OmMSet::convert_to_percent1(wt)
    om_weight   wt
    CODE:
        THIS->convert_to_percent(wt);

void
OmMSet::convert_to_percent2(it)
    OmMSetIterator *    it
    CODE:
        THIS->convert_to_percent(* it);

om_doccount
OmMSet::get_termfreq(om_termname tname)

om_weight
OmMSet::get_termweight(om_termname tname)

om_doccount
OmMSet::get_firstitem()

om_doccount
OmMSet::get_matches_lower_bound()

om_doccount
OmMSet::get_matches_estimated()

om_doccount
OmMSet::get_matches_upper_bound()

om_weight
OmMSet::get_max_possible()

om_weight
OmMSet::get_max_attained()

om_doccount
OmMSet::size()

om_doccount
OmMSet::max_size()

bool
OmMSet::empty()

void
OmMSet::swap(other)
        OmMSet *    other
    CODE:
        THIS->swap(* other);

OmMSetIterator *
OmMSet::begin()
    CODE:
        RETVAL = new OmMSetIterator();
        *RETVAL = THIS->begin();
    OUTPUT:
        RETVAL

OmMSetIterator *
OmMSet::end()
    CODE:
        RETVAL = new OmMSetIterator();
        *RETVAL = THIS->end();
    OUTPUT:
        RETVAL

OmMSetIterator *
OmMSet::back()
    CODE:
        RETVAL = new OmMSetIterator();
        *RETVAL = THIS->back();
    OUTPUT:
        RETVAL

OmMSetIterator *
OmMSet::get_msetiterator(om_doccount i)
    CODE:
        RETVAL = new OmMSetIterator();
        *RETVAL = (*THIS)[i];
    OUTPUT:
        RETVAL

string
OmMSet::get_description()

void
OmMSet::DESTROY()
