/* querygui.cc: Test GUI for om
 *
 * ----START-LICENCE----
 * Copyright 1999,2000 BrightStation PLC
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

#include "config.h"

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <cstdio>
#include <string>
#include <algorithm>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include "querygui.h"

#include <om/om.h>

#include "../indexer/index_utils.h"

#include <list>
#include <memory>

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
	ResultItemGTK(om_docid did_new, 
		      int percent,
		      om_docname dname) : did(did_new)
	{
	    data = new gchar *[3];
	    data[0] = c_string(inttostring(percent));
	    data[1] = c_string(inttostring(did_new));
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
	om_docid did = mset.items[row].did;
	
	OmDocument doc(enquire->get_doc(mset.items[row]));
	string fulltext = doc.get_data().value;
	
	string score = inttostring(mset.convert_to_percent(mset.items[row]));

	gtk_text_freeze(result_text);
	gtk_text_backward_delete(result_text, gtk_text_get_length(result_text));
	gtk_text_insert(result_text, NULL, NULL, NULL, fulltext.c_str(), -1);
	gtk_text_thaw(result_text);
	gtk_label_set_text(result_query, querystring.c_str());
	gtk_label_set_text(result_score, score.c_str());
	gtk_label_set_text(result_docid, inttostring(did).c_str());
    } catch (OmError &e) {
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

	if (!rset.items.size()) {
	    // invent an rset
	    gint msize = results_widget->rows;
	    for (index = min(4, msize - 1); index >= 0; index--) {
		gpointer rowdata = gtk_clist_get_row_data(results_widget, index);
		ResultItemGTK * item = (ResultItemGTK *) rowdata;
		rset.add_document(item->did);
	    }
	}

	OmESet topterms = enquire->get_eset(50, rset);
	//topterms.expand(&rset, &decider);

	gtk_clist_freeze(topterms_widget);
	gtk_clist_clear(topterms_widget);

	vector<OmESetItem>::const_iterator i;
	for (i = topterms.items.begin(); i != topterms.items.end(); i++) {
	    string tname = i->tname;
//#ifdef DEBUG
	    tname = tname + " (" + floattostring(i->wt) + ")";
//#endif

	    TopTermItemGTK * item = new TopTermItemGTK(tname);
	    gint index = gtk_clist_append(topterms_widget, item->data);

	    // Make sure it gets freed when item is removed from result list
	    gtk_clist_set_row_data_full(topterms_widget, index, item,
					topterm_destroy_notify);
	}
    } catch (OmError &e) {
	cout << e.get_msg() << endl;
    }
    gtk_clist_thaw(topterms_widget);
}

static void
on_results_selection(GtkWidget *widget,
		     gint row,
		     gint column,
		     GdkEventButton *event,
		     gpointer data)
{
    do_topterms();
    do_resultdisplay(row);
}

static void
on_query_changed(GtkWidget *widget, gpointer user_data) {
    GtkEditable *textbox = GTK_EDITABLE(widget);
    char *tmp = gtk_editable_get_chars( textbox, 0, -1);
    querystring = string(tmp);
    g_free(tmp);

    try {
	// split into terms
	OmQuery omquery;
	OmStem stemmer("english");
	om_termname word;
        string::size_type spacepos;
	om_termcount position = 1;
	string unparsed_query = querystring;
	while((spacepos = unparsed_query.find_first_not_of(" \t\n")) != string::npos) {
	    if(spacepos) unparsed_query = unparsed_query.erase(0, spacepos);
	    spacepos = unparsed_query.find_first_of(" \t\n");
	    word = unparsed_query.substr(0, spacepos);
	    select_characters(word, "");
	    lowercase_term(word);
	    word = stemmer.stem_word(word);
	    omquery = OmQuery(OM_MOP_OR, omquery, OmQuery(word, 1, position++));
	    unparsed_query = unparsed_query.erase(0, spacepos);
	}

	// Perform match
	enquire->set_query(omquery);
        mset = enquire->get_mset(0, max_msize);

	gtk_clist_freeze(results_widget);
	gtk_clist_clear(results_widget);
	cout << "MBound: " << mset.mbound <<
	        " Max_possible: " << mset.max_possible <<
	        " Max_attained: " << mset.max_attained << endl;

	vector<OmMSetItem>::const_iterator j;
	for (j = mset.items.begin(); j != mset.items.end(); j++) {
	    om_termname_list mterms = enquire->get_matching_terms(*j);
	    vector<string> sorted_mterms(mterms.begin(), mterms.end());
	    string message;
	    for (vector<string>::const_iterator i = sorted_mterms.begin();
		 i != sorted_mterms.end();
		 ++i) {
		if (message.size() > 0) message += " ";

		message += *i;
	    }

	    ResultItemGTK * item = new ResultItemGTK(j->did,
		mset.convert_to_percent(*j), message);
	    gint index = gtk_clist_append(results_widget, item->data);

	    // Make sure it gets freed when item is removed from result list
	    gtk_clist_set_row_data_full(results_widget, index, item,
					result_destroy_notify);
	}
	gtk_clist_thaw(results_widget);
	do_topterms();
    } catch (OmError &e) {
	gtk_clist_thaw(results_widget);
	cout << e.get_msg() << endl;
    }
}

static gboolean
on_mainwindow_destroy(GtkWidget *widget,
		      GdkEvent *event,
		      gpointer user_data) {
    gtk_main_quit();
    return FALSE;
}

int main(int argc, char *argv[]) {
    string gladefile = "querygui.glade";
    enquire = NULL;
    max_msize = 10;
    vector<string> dbtypes;
    vector<vector<string> > dbargs;

    gtk_init(&argc, &argv);
    glade_init();

    const char *progname = argv[0];

    bool syntax_error = false;
    argv++;
    argc--;
    while (argc && argv[0][0] == '-') {
	if (argc >= 2 && strcmp(argv[0], "--max-msize") == 0) {
	    max_msize = atoi(argv[1]);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--da") == 0) {
	    dbtypes.push_back("da_flimsy");
	    vector<string> args;
	    args.push_back(argv[1]);
	    dbargs.push_back(args);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--im") == 0) {
	    dbtypes.push_back("inmemory");
	    vector<string> args;
	    args.push_back(argv[1]);
	    dbargs.push_back(args);
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

    if (!dbtypes.size() || syntax_error || argc >= 1) {
	cout << "Syntax: " << progname << " [options]" << endl;
	cout << "\t--max-msize <maximum msize>\n";
	cout << "\t--da <DA directory>\n";
	cout << "\t--im <textfile>\n";
	cout << "\t--glade <glade interface definition file>\n";
	exit(1);
    }

    // Set Database(s)
    try {
	OmDatabaseGroup mydbs;

	vector<string>::const_iterator p;
	vector<vector<string> >::const_iterator q;
	for(p = dbtypes.begin(), q = dbargs.begin();
	    p != dbtypes.end();
	    p++, q++) {
	    mydbs.add_database(*p, *q);
	}

	GladeXML *xml;

	/* load the interface */
	struct stat statbuf;
	int err = stat(gladefile.c_str(), &statbuf);
	if(err) {
	    cerr << "Unable to open " << gladefile <<
		    " (" << strerror(errno) << ")" << endl;
	    return 1;
	}

	xml = glade_xml_new(gladefile.c_str(), NULL);
	if(xml == NULL) {
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

    } catch (OmError &e) {
	cout << "OmError: " << e.get_msg() << endl;
    }
    delete enquire;
    enquire = NULL;

    return 0;
}
