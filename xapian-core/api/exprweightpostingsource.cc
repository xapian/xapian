/** @file exprweightpostingsource.cc
 * @brief Expression evaluator source of posting information
 */
/* Copyright (C) 2008 Lemur Consulting Ltd
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

#include <xapian.h>
#include "database.h"
#include "document.h"
#include "xapian/document.h"
#include "xapian/queryparser.h" // For sortable_unserialise
#include "autoptr.h"
#include "xapian/exprweightpostingsource.h"
#include "omassert.h"
#include "serialise.h"
#include "serialise-double.h"
#include "gammonparser.h"
#include <iostream>

namespace Xapian {

ExprWeightPostingSource::ExprWeightPostingSource(PropertyMap prop_map_,
                                                 DefaultMap default_map_,
                                                 Xapian::weight max_weight_)
        : current_docid(0),
          last_docid(db.get_lastdocid()),
          termfreq_max(db.get_doccount()),
          expression(),
          max_weight(max_weight_),
          current_weight(0.0),
          prop_map(prop_map_),
          default_map(default_map_)
{
}

void
ExprWeightPostingSource::set_expression(std::string expression_)
{
    expression = expression_;
}

Xapian::doccount
ExprWeightPostingSource::get_termfreq_min() const
{
    return 0;
}

Xapian::doccount
ExprWeightPostingSource::get_termfreq_est() const
{
    return termfreq_max;
}

Xapian::doccount
ExprWeightPostingSource::get_termfreq_max() const
{
    return termfreq_max;
}

Xapian::weight
ExprWeightPostingSource::get_maxweight() const
{
    return max_weight;
}

Xapian::weight
ExprWeightPostingSource::get_weight() const
{
    Assert(! at_end());
    Assert(current_docid != 0);
    return current_weight;
}

void
ExprWeightPostingSource::next(Xapian::weight min_wt)
{
    if (min_wt > max_weight)
    {
    current_docid = last_docid + 1;
    return;
    }

    while (current_docid <= last_docid)
    {
    ++current_docid;
    if (current_docid > last_docid) return;

    Parser parser(expression);
        try
        {
            Xapian::Document doc = db.get_document(current_docid);
            if (! get_property_values(doc, parser)) continue;
        }
        catch (const Xapian::DocNotFoundError &)
        {
            continue;
        }

        // evaluate the expression
        current_weight = parser.Evaluate();
        // Don't check that the value is > min_wt, since this
        // could be a slow loop and isn't required.
        
        std::cout << "## current docid and weight: " << current_docid << " : " 
            << current_weight << std::endl;
        
        if (current_weight < 0.0) current_weight = 0.0;
        if (current_weight > max_weight) current_weight = max_weight;
        return;
    }
}

bool
ExprWeightPostingSource::get_property_values(const Xapian::Document& doc, Parser& parser)
{
    for (PropertyMap::const_iterator it = prop_map.begin(); it != prop_map.end(); ++it)
    {
        if (expression.find(it->first) == std::string::npos) continue;
        std::string value = doc.get_value(it->second);
        if (value.empty())
        {
            DefaultMap::const_iterator find = default_map.find(it->first);
            if (find == default_map.end()) return false;
            parser[it->first] = find->second;
        }
        else
        {
            parser[it->first] = sortable_unserialise(value);
        }
    }
    return true;
}

void
ExprWeightPostingSource::skip_to(Xapian::docid min_docid,
                                 Xapian::weight min_wt)
{
    if (current_docid < min_docid)
    {
    current_docid = min_docid - 1;
    next(min_wt);
    }
}

bool
ExprWeightPostingSource::at_end() const
{
    return current_docid > last_docid;
}

Xapian::docid
ExprWeightPostingSource::get_docid() const
{
    return current_docid;
}

ExprWeightPostingSource *
ExprWeightPostingSource::clone() const
{
    ExprWeightPostingSource * cl = new ExprWeightPostingSource(prop_map, default_map, max_weight);
    cl->set_expression(expression);
    return cl;
}

std::string
ExprWeightPostingSource::name() const
{
    return std::string("ExprWeight");
}

std::string
ExprWeightPostingSource::serialise() const
{
    std::string result;
    result += encode_length(prop_map.size());
    for (PropertyMap::const_iterator it = prop_map.begin(); it != prop_map.end(); ++it)
    {
        result += encode_length(it->first.length());
        result += it->first;
        result += encode_length(it->second);
    }
    result += encode_length(default_map.size());
    for (DefaultMap::const_iterator it = default_map.begin(); it != default_map.end(); ++it)
    {
        result += encode_length(it->first.length());
        result += it->first;
        result += serialise_double(it->second);
    }
    result += serialise_double(max_weight);
    result += expression;
    return result;
}

PostingSource *
ExprWeightPostingSource::unserialise(const std::string &s) const
{
    // Use c_str() so last string is null-terminated.
    const char * p = s.c_str();
    const char * end = p + s.size();
    size_t map_size;

    map_size = decode_length(&p, end, true);
    PropertyMap new_prop_map;
    for (; map_size > 0; --map_size)
    {
        size_t len = decode_length(&p, end, true);
        std::string prop(p, len);
        p += len;
        new_prop_map[prop] = decode_length(&p, end, true);
    }

    map_size = decode_length(&p, end, true);
    DefaultMap new_default_map;
    for (; map_size > 0; --map_size)
    {
        size_t len = decode_length(&p, end, true);
        std::string prop(p, len);
        p += len;
        new_default_map[prop] = unserialise_double(&p, end);
    }

    Xapian::weight new_max_weight = unserialise_double(&p, end);
    std::string expr(p);

    ExprWeightPostingSource * new_source = new ExprWeightPostingSource(new_prop_map, new_default_map, new_max_weight);
    new_source->set_expression(expr);
    return new_source;
}

void
ExprWeightPostingSource::reset(const Database &db_)
{
    db = db_;
    current_docid = 0;
    last_docid = db.get_lastdocid();
    termfreq_max = db.get_doccount();
    current_weight = 0.0;
}

std::string
ExprWeightPostingSource::get_description() const
{
    return "Xapian::ExprWeightPostingSource(" + expression + ")";
}

}
