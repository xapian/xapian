/************************************************************
 *
 *  aligned_diff.h is a special type of diff, its output
 *  consists of adds, deletes, and 1-1 changes 
 *  (e.g. 3 lines changed to new 3 lines)
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
 *  Usage:
 *
 *  suppose cin contain output from a diff.
 *  aligned_diff d;
 *  cin >> d;
 *  cout << d; // outputs modified so all changed lines are 
 *             // aligned up.
 *
 *  $Id$
 *
 ************************************************************/

#ifndef __ALIGNED_DIFF_H__
#define __ALIGNED_DIFF_H__

#include "diff.h"

class aligned_diff : public diff
{
protected:
    virtual istream & read(istream &);
};

#endif
