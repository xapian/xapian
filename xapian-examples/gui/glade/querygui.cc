/* querygui.cc: Test GUI for om
 *
 * ----START-LICENCE----
 * Copyright 1999 Dialog Corporation
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
#include <unistd.h>
#include <sys/stat.h>
#include "querygui.h"

#include "om.h"

#include "textfile_indexer.h"
#include "query_parser.h"

#include <list>

IRDatabase *database;
Match * matcher;
vector<MSetItem> mset;
string query;

doccount max_msize;
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
	TopTermItemGTK(termname &tname_new) : tname(tname_new)
	{
	    data = new gchar *[1];
	    data[0] = c_string(tname);
	}
	~TopTermItemGTK() {
	    delete data[0];
	    delete data;
	}
	termname tname;
	
	gchar **data;
};

class ResultItemGTK {
    public:
	ResultItemGTK(docid did_new, int percent, docname dname) : did(did_new)
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
	docid did;
	
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
	docid did = mset[row].did;
	IRDocument *doc = database->open_document(did);
	IRData data = doc->get_data();
	string fulltext = data.value;
	weight maxweight = matcher->get_max_weight();
	string score = inttostring((int)(100 * mset[row].wt / maxweight));

	gtk_text_freeze(result_text);
	gtk_text_backward_delete(result_text, gtk_text_get_length(result_text));
	gtk_text_insert(result_text, NULL, NULL, NULL, fulltext.c_str(), -1);
	gtk_text_thaw(result_text);
	gtk_label_set_text(result_query, query.c_str());
	gtk_label_set_text(result_score, score.c_str());
	gtk_label_set_text(result_docid, inttostring(did).c_str());
    } catch (OmError e) {
	cout << e.get_msg() << endl;
    }
}

static void do_topterms() {
    try {
	RSet rset(database);
	GList *next = results_widget->selection;
	gint index;

	while(next) {
	    index = GPOINTER_TO_INT(next->data);
	    gpointer rowdata = gtk_clist_get_row_data(results_widget, index);
	    ResultItemGTK * item = (ResultItemGTK *) rowdata;
	    rset.add_document(item->did);
	    next = next->next;
	}

	if (!rset.get_rsize()) {
	    // invent an rset
	    gint msize = results_widget->rows;
	    for (index = min(4, msize - 1); index >= 0; index--) {
		gpointer rowdata = gtk_clist_get_row_data(results_widget, index);
		ResultItemGTK * item = (ResultItemGTK *) rowdata;
		rset.add_document(item->did);
	    }
	}

	Expand topterms(database);
	ExpandDeciderAlways decider;
	topterms.expand(&rset, &decider);

	gtk_clist_freeze(topterms_widget);
	gtk_clist_clear(topterms_widget);

	vector<ESetItem>::const_iterator i;
	for (i = topterms.eset.begin(); i != topterms.eset.end(); i++) {
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
    } catch (OmError e) {
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
    query = string(tmp);
    g_free(tmp);

    try {
	delete matcher;
	matcher = NULL;
	matcher = new Match(database); 

	// split into terms
	QueryParser parser;
	TextfileIndexer idx;
	parser.set_indexer(&idx);
	vector<QueryTerm> qterms;
	qterms = parser.parse_query(query);

	vector<QueryTerm>::const_iterator i = qterms.begin();
	while(i != qterms.end()) {
	    matcher->add_term((*i).tname);
	    i++;
	}

	// Perform match
	doccount mtotal;
	matcher->match(0, max_msize, mset, &mtotal);
	weight maxweight = matcher->get_max_weight();

	gtk_clist_freeze(results_widget);
	gtk_clist_clear(results_widget);
	cout << "MTotal: " << mtotal << " Maxweight: " << maxweight << endl;
	vector<MSetItem>::const_iterator j;
	for (j = mset.begin(); j != mset.end(); j++) {
	    docid did = j->did;
	    IRDocument *doc = database->open_document(did);
	    IRData data = doc->get_data();
	    string message;
#if 0
	    if(Database *mdb = dynamic_cast<MultiDatabase *>(database)) {
		message = (mdb->get_database_of_doc(did))->get_database_path();
	    } else {
		message = database->get_database_path();
	    }
	    message += " ";
#endif
	    message += data.value;
	    message = data.value;
	    ResultItemGTK * item = new ResultItemGTK(j->did,
		100 * j->wt / maxweight, message);
	    gint index = gtk_clist_append(results_widget, item->data);

	    // Make sure it gets freed when item is removed from result list
	    gtk_clist_set_row_data_full(results_widget, index, item,
					result_destroy_notify);
	}
    } catch (OmError e) {
	cout << e.get_msg() << endl;
    }
    gtk_clist_thaw(results_widget);
    do_topterms();
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
    max_msize = 10;
    list<string> dbnames;
    list<om_database_type> dbtypes;

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
	    dbnames.push_back(argv[1]);
	    dbtypes.push_back(OM_DBTYPE_DA);
	    argc -= 2;
	    argv += 2;
	} else if (argc >= 2 && strcmp(argv[0], "--im") == 0) {
	    dbnames.push_back(argv[1]);
	    dbtypes.push_back(OM_DBTYPE_INMEMORY);
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

    if (syntax_error || argc >= 1) {
	cout << "Syntax: " << progname << " [options]" << endl;
	cout << "\t--max-msize <maximum msize>\n";
	cout << "\t--da <DA directory>\n";
	cout << "\t--im <textfile>\n";
	cout << "\t--glade <glade interface definition file>\n";
	exit(1);
    }

    if(!dbnames.size()) {
	dbnames.push_back("/mnt/ivory/disk1/home/richard/textfile");
	dbtypes.push_back(OM_DBTYPE_INMEMORY);
    }

    // Prepare to open database
    DatabaseBuilderParams dbparams;
    if (dbnames.size() > 1) {
	dbparams.type = OM_DBTYPE_MULTI;

	list<string>::const_iterator p;
	list<om_database_type>::const_iterator q;
	for(p = dbnames.begin(), q = dbtypes.begin();
	    p != dbnames.end();
	    p++, q++) {
	    DatabaseBuilderParams subparams(*q);
	    subparams.paths.push_back(*p);
	    dbparams.subdbs.push_back(subparams);
	}
    } else {
	dbparams.type = *(dbtypes.begin());
	dbparams.paths.push_back(*(dbnames.begin()));
    }

    // Actually open the database
    try {
	database = DatabaseBuilder::create(dbparams);
    } catch (OmError e) {
	cout << e.get_msg() << endl;
	exit(1);
    }

    matcher = new Match(database); 

    // FIXME - debugging code - remove this
    matcher->add_term("love");
    doccount mtotal;
    matcher->match(0, 10, mset, &mtotal);
    weight maxweight = matcher->get_max_weight();
    cout << maxweight << " " << mtotal << " " << mset.size() << endl;

    delete matcher;
    matcher = NULL;

    GladeXML *xml;

    /* load the interface */

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

    /* start the event loop */
    gtk_main();
    return 0;
}
