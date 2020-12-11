/** @file
 * @brief Wrapper around standard unique_ptr template.
 */
/* Copyright (C) 2009,2011,2014,2015 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

// This wrapper now only exists to make it easier to backport patches to 1.2.x
// where AutoPtr is defined to std::auto_ptr.  This wrapper can be dropped when
// 1.2.x is retired, or once we get to the stage of only backporting fixes for
// serious bugs.
#ifndef AutoPtr
# include <memory>
# define AutoPtr std::unique_ptr
#endif
