/************************************************************
 *
 * cvs_output contains strings output by cvs systems.
 * These strings are used to parse cvs outputs.
 *
 * This file is a part of the cvssearch library.
 * 
 * $Id$
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

    static string cvs_diff_separator;
};

#endif
