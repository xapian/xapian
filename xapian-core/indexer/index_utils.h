/* index_utils.h
 */

#ifndef _index_utils_h_
#define _index_utils_h_

#include "omtypes.h"
void lowercase_term(termname &);
void select_characters(termname &term, const string & keep);
void get_paragraph(istream &input, string &para);
void get_a_line(istream &input, string &line);

#endif /* _index_utils_h_ */
