%module(directors="1") xapian

#define XAPIAN_SWIG_DIRECTORS

%rename(Apply) operator();
%ignore operator&;
%ignore operator|;
%ignore operator^;
%ignore operator*;
%ignore operator/;

%ignore Xapian::Compactor::resolve_duplicate_metadata(std::string const &key, size_t num_tags, std::string const tags[]);

%include ../xapian-head.i

//%include ../generic/except.i
%include ../xapian-headers.i

