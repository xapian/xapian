/* xapview.cc: Simple GUI for xapian
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
 * Copyright 2001 Lemur Consulting Ltd
 * Copyright 2001,2002 Ananova Ltd
 * Copyright 2002 Olly Betts
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 * -----END-LICENCE-----
 */

#include <config.h>

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <cstdio>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include <om/om.h>

#include <algorithm>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <vector>

using namespace std;

// C prototypes for callback functions which have to have C style naming.
extern "C" {
    void on_query_changed(GtkEditable*,gpointer);
    void on_results_selection(GtkCList*,gint,gint,GdkEventButton*,gpointer);
}


// FIXME: these 2 copied from om/indexer/index_utils.cc
static void
lowercase_term(om_termname &term)
{
    for (om_termname::iterator i = term.begin(); i != term.end(); ++i) {
	*i = tolower(*i);
    }
}

// Keep only the characters in keep
static void
select_characters(om_termname &term)
{
    const string chars(
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
    string::size_type pos;
    while ((pos = term.find_first_not_of(chars)) != string::npos) {
	string::size_type endpos = term.find_first_of(chars, pos);
	term.erase(pos, endpos - pos);
    }
}

OmEnquire * enquire;
OmMSet mset;
string querystring;

om_doccount max_msize;
GtkCList *results_widget;
GtkCList *topterms_widget;
GtkLabel *result_query;
GtkLabel *result_score;
GtkLabel *result_docid;
GtkText *result_text;

gchar *
c_string(const string & s)
{
    gchar * p = new gchar[s.length() + 1];
    s.copy(p, string::npos);
    p[s.length()] = '\0';
    return p;
}

// Convert an integer to a string
#include <strstream.h>
string inttostring(int a)
{
    // Use ostrstream (because ostringstream often doesn't exist)
    char buf[100];  // Very big (though we're also bounds checked)
    ostrstream ost(buf, 100);
    ost << a << ends;
    return string(buf);
}

string floattostring(double a)
{
    // Use ostrstream (because ostringstream often doesn't exist)
    char buf[100];  // Very big (though we're also bounds checked)
    ostrstream ost(buf, 100);
    ost << a << ends;
    return string(buf);
}

class TopTermItemGTK {
    public:
	TopTermItemGTK(om_termname &tname_new) : tname(tname_new)
	{
	    data = new gchar *[1];
	    data[0] = c_string(tname);
	}
	~TopTermItemGTK() {
	    delete data[0];
	    delete data;
	}
	om_termname tname;
	
	gchar **data;
};

class ResultItemGTK {
    public:
	ResultItemGTK(om_docid did_, int percent, string dname) : did(did_)
	{
	    data = new gchar *[3];
	    data[0] = c_string(inttostring(percent));
	    data[1] = c_string(inttostring(did));
	    data[2] = c_string(dname);
	}
	~ResultItemGTK() {
	    delete data[0];
	    delete data[1];
	    delete data[2];
	    delete data;
	}
	om_docid did;
	
	gchar **data;
};

static void
topterm_destroy_notify(gpointer data)
{
    TopTermItemGTK * item = (TopTermItemGTK *)data;
    delete item;
}

static void
result_destroy_notify(gpointer data)
{
    ResultItemGTK * item = (ResultItemGTK *)data;
    delete item;
}

static void do_resultdisplay(gint row) {
    try {
	OmMSetIterator i = mset[row];
	
	OmDocument doc = i.get_document();
	string fulltext = doc.get_data();
	
	string score = inttostring(mset.convert_to_percent(i));

	gtk_text_freeze(result_text);
	gtk_text_backward_delete(result_text, gtk_text_get_length(result_text));
	gtk_text_insert(result_text, NULL, NULL, NULL, fulltext.c_str(), -1);
	gtk_text_thaw(result_text);
	gtk_label_set_text(result_query, querystring.c_str());
	gtk_label_set_text(result_score, score.c_str());
	gtk_label_set_text(result_docid, inttostring(*i).c_str());
    } catch (const OmError &e) {
	cout << e.get_msg() << endl;
    }
}

static void do_topterms() {
    try {
	OmRSet rset;
	GList *next = results_widget->selection;
	gint index;

	while(next) {
	    index = GPOINTER_TO_INT(next->data);
	    gpointer rowdata = gtk_clist_get_row_data(results_widget, index);
	    ResultItemGTK * item = (ResultItemGTK *) rowdata;
	    rset.add_document(item->did);
	    next = next->next;
	}

	if (!rset.size()) {
	    // invent an rset
	    gint msize = results_widget->rows;
	    for (index = min(4, msize - 1); index >= 0; index--) {
		gpointer rowdata;
		rowdata = gtk_clist_get_row_data(results_widget, index);
		ResultItemGTK * item = (ResultItemGTK *) rowdata;
		rset.add_document(item->did);
	    }
	}

	OmESet topterms = enquire->get_eset(50, rset);
	//topterms.expand(&rset, &decider);

	gtk_clist_freeze(topterms_widget);
	gtk_clist_clear(topterms_widget);
	
	for (OmESetIterator i = topterms.begin(); i != topterms.end(); i++) {
	    string tname = *i;
	    tname = tname + " (" + floattostring(i.get_weight()) + ")";

	    TopTermItemGTK * item = new TopTermItemGTK(tname);
	    gint index = gtk_clist_append(topterms_widget, item->data);

	    // Make sure it gets freed when item is removed from result list
	    gtk_clist_set_row_data_full(topterms_widget, index, item,
					topterm_destroy_notify);
	}
    } catch (const OmError &e) {
	cout << e.get_msg() << endl;
    }
    gtk_clist_thaw(topterms_widget);
}

void
on_results_selection(GtkCList *clist,
		     gint row,
		     gint column,
		     GdkEventButton *event,
		     gpointer data)
{
    do_topterms();
    do_resultdisplay(row);
}

void
on_query_changed(GtkEditable *editable, gpointer user_data)
{
    GtkEditable *textbox = editable;
    char *tmp = gtk_editable_get_chars( textbox, 0, -1);
    querystring = string(tmp);
    g_free(tmp);

    try {
	// split into terms
	OmQuery omquery;
	OmStem stemmer("english");
	om_termname word;
	om_termcount position = 1;
	string unparsed_query = querystring;
	while (true) {
	    string::size_type spacepos;
	    spacepos = unparsed_query.find_first_not_of(" \t\n");
	    if (spacepos == string::npos) break;
	    if (spacepos) unparsed_query = unparsed_query.erase(0, spacepos);
	    spacepos = unparsed_query.find_first_of(" \t\n");
	    word = unparsed_query.substr(0, spacepos);
	    select_characters(word);
	    lowercase_term(word);
	    word = stemmer.stem_word(word);
	    if (!omquery.is_empty()) {
		omquery = OmQuery(OmQuery::OP_OR, omquery,
				                  OmQuery(word, 1, position++));
	    } else {
		omquery = OmQuery(word, 1, position++);
	    }
	    unparsed_query = unparsed_query.erase(0, spacepos);
	}

	// Perform match
	enquire->set_query(omquery);
        mset = enquire->get_mset(0, max_msize);

	gtk_clist_freeze(results_widget);
	gtk_clist_clear(results_widget);
#if 0
	cout << "matches_lower_bound: " << mset.get_matches_lower_bound() <<
		" matches_estimated: " << mset.get_matches_estimated() <<
		" matches_upper_bound: " << mset.get_matches_upper_bound() <<
	        " max_possible: " << mset.get_max_possible() <<
	        " max_attained: " << mset.get_max_attained() << endl;
#endif

	for (OmMSetIterator j = mset.begin(); j != mset.end(); ++j) {
	    vector<string> sorted_mterms(
	    	enquire->get_matching_terms_begin(j),
		enquire->get_matching_terms_end(j));
	    string message;
	    vector<string>::const_iterator i;
	    for (i = sorted_mterms.begin(); i != sorted_mterms.end(); ++i) {
		if (!message.empty()) message += ' ';
		message += *i;
	    }

	    ResultItemGTK * item = new ResultItemGTK(*j,
		mset.convert_to_percent(j), message);
	    gint index = gtk_clist_append(results_widget, item->data);

	    // Make sure it gets freed when item is removed from result list
	    gtk_clist_set_row_data_full(results_widget, index, item,
					result_destroy_notify);
	}
	gtk_clist_thaw(results_widget);
	do_topterms();
    } catch (const OmError &e) {
	gtk_clist_thaw(results_widget);
	cout << e.get_msg() << endl;
    }
}

gboolean
on_mainwindow_destroy(GtkWidget *widget,
		      GdkEvent *event,
		      gpointer user_data) {
    gtk_main_quit();
    return FALSE;
}

int main(int argc, char *argv[]) {
    string gladefile = "xapview.glade";
    enquire = NULL;
    max_msize = 10;
    vector<OmSettings> dbs;

    gtk_init(&argc, &argv);
    glade_init();

    const char *progname = argv[0];

    bool syntax_error = false;
    argv++;
    argc--;
    // FIXME: use getopt?  or make these settable from the GUI...
    while (argc && argv[0][0] == '-') {
	if (argc >= 2 && strcmp(argv[0], "--msize") == 0) {
	    max_msize = atoi(argv[1]);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--dbdir") == 0) {
	    OmSettings params;
	    params.set("backend", "auto");
	    params.set("auto_dir", argv[1]);
	    dbs.push_back(params);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--glade") == 0) {
	    gladefile = argv[1];
	    argc -= 2;
	    argv += 2;
	} else {
	    syntax_error = true;
	    break;
	}
    }

    if (dbs.empty() || syntax_error || argc >= 1) {
	cout << "Syntax: " << progname << " [options]\n"
	     << "\t--msize <maximum msize>\n"
	     << "\t--dbdir <directory>\n"
	     << "\t--glade <glade interface definition file>" << endl;
	exit(1);
    }

    // Set Database(s)
    try {
	OmDatabase mydbs;

	vector<OmSettings>::const_iterator p;
	for (p = dbs.begin(); p != dbs.end(); p++) {
	    mydbs.add_database(*p);
	}

	GladeXML *xml;

	/* load the interface */
	struct stat statbuf;
	int err = stat(gladefile.c_str(), &statbuf);
	if (err) {
	    cerr << "Unable to open " << gladefile
		 << " (" << strerror(errno) << ")" << endl;
	    return 1;
	}

	xml = glade_xml_new(gladefile.c_str(), NULL);
	if (xml == NULL) {
	    cerr << "Unable to open " << gladefile << endl;
	    return 1;
	}

	/* connect the signals in the interface */
	glade_xml_signal_autoconnect(xml);

	topterms_widget = GTK_CLIST(glade_xml_get_widget(xml, "topterms"));
	results_widget = GTK_CLIST(glade_xml_get_widget(xml, "results"));
	result_query = GTK_LABEL(glade_xml_get_widget(xml, "result_query"));
	result_score = GTK_LABEL(glade_xml_get_widget(xml, "result_score"));
	result_docid = GTK_LABEL(glade_xml_get_widget(xml, "result_docid"));
	result_text = GTK_TEXT(glade_xml_get_widget(xml, "result_text"));

	// Start enquiry system
	enquire = new OmEnquire(mydbs);

	/* start the event loop */
	gtk_main();
    } catch (const OmError &e) {
	cout << "OmError: " << e.get_msg() << endl;
    }
    delete enquire;
    enquire = NULL;

    return 0;
}
