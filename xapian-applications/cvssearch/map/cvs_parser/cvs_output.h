/************************************************************
 *
 *  cvs_output.h contains strings output by cvs systems,
 *  these strings are used to parse cvs outputs.
 *
 *  (c) 2001 Andrew Yao (andrewy@users.sourceforge.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  $Id$
 *
 ************************************************************/

#ifndef __CVS_OUTPUT_H__
#define __CVS_OUTPUT_H__

#include <string>
using std::string;

class cvs_output
{
public:
    static string cvs_log_separator;
    static string cvs_log_end_marker;
    static string cvs_log_filename_tag;
    static string cvs_log_rcs_file_tag;
    static string cvs_log_revision_tag;
    static string cvs_log_branches_tag;
    static string cvs_log_empty_comment;
    static string cvs_log_date_tag;
    static string cvs_log_author_tag;
    static string cvs_log_state_tag;
    static string cvs_log_lines_tag;
    static string cvs_diff_separator;
};

#endif
