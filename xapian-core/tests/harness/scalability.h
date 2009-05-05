/** @file scalability.h
 * @brief Test how an operation scales.
 */
/* Copyright (C) 2009 Olly Betts
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

#ifndef XAPIAN_INCLUDED_SCALABILITY_H
#define XAPIAN_INCLUDED_SCALABILITY_H

void test_scalability(double (*func)(unsigned), unsigned n, double threshold);

// 10 times the work should increase the time by:
// O(1) : 1 
// O(n) : 10
// O(n*log(n)) : 20 = 100 * log(100) / (10 * log(10))
// O(n*n) : 100

#define O_N 14.1
#define O_N_LOG_N 44.7

#endif // XAPIAN_INCLUDED_SCALABILITY_H
