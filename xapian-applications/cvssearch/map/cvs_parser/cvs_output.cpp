/************************************************************
 *
 * cvs_output strings output by cvs systems.
 *
 * This file is a part of the cvssearch library.
 * 
 * $Id$
 *
 ************************************************************/

#include "cvs_output.h"

string cvs_output::cvs_log_separator = "----------------------------";
string cvs_output::cvs_log_revision_tag = "revision ";
string cvs_output::cvs_log_end_marker = "=============================================================================";
string cvs_output::cvs_log_filename_tag = "Working file: ";
string cvs_output::cvs_log_branches_tag = "branches:";
string cvs_output::cvs_log_empty_comment= "*** empty log message ***";
string cvs_output::cvs_log_rcs_file_tag = "RCS file: ";
string cvs_output::cvs_diff_separator = "---";
string cvs_output::cvs_log_date_tag = "date: ";
string cvs_output::cvs_log_author_tag =";  author: ";
string cvs_output::cvs_log_state_tag = ";  state: ";
string cvs_output::cvs_log_lines_tag = ";  lines: ";
