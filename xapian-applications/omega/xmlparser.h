/** @file
 * @brief XML (and HTML) parser
 */
/* Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2002,2006,2008,2009,2011,2016,2020 Olly Betts
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
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef OMEGA_INCLUDED_XMLPARSER_H
#define OMEGA_INCLUDED_XMLPARSER_H

#include <string>

class XmlParser {
    const char* attribute_data;
    mutable size_t attribute_len;

  protected:
    /** Control HTML-specific handling.
     *
     *  Defaults to XML, which means no HTML-specific handling.
     *
     *  The HtmlParser subclass overrides this to HTML at construction time,
     *  and then it can change to HTML_IN_SCRIPT and back to HTML as we
     *  move in and out of parsing script elements.
     */
    enum { XML, HTML, HTML_IN_SCRIPT } state = XML;

    std::string charset;

    /// Protected constructor for HtmlParser subclass.
    explicit XmlParser(bool) : state(HTML) { }

    static void decode_entities(std::string& s);

    bool get_attribute(const std::string& name, std::string& value) const;

    /** Process an opening tag.
     *
     *  Return false to stop parsing of the rest of the document.
     */
    virtual bool opening_tag(const std::string& tag) {
	(void)tag;
	return true;
    }

    /** Process a closing tag.
     *
     *  Return false to stop parsing of the rest of the document.
     */
    virtual bool closing_tag(const std::string& tag) {
	(void)tag;
	return true;
    }

    /// Process text between tags.
    virtual void process_content(const std::string& content) {
	(void)content;
    }

  public:
    XmlParser() { }

    XmlParser(const XmlParser&) = delete;

    XmlParser& operator=(const XmlParser&) = delete;

    virtual ~XmlParser() { }

    void parse(const std::string& text);
};

#endif // OMEGA_INCLUDED_XMLPARSER_H
