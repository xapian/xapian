/* querygui.cc */

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <cstdio>
#include <string>
#include "querygui.h"

#include "database.h"
#include "postlist.h"
#include "termlist.h"
#include "irdocument.h"
#include "match.h"
#include "stem.h"
#include "textfile_database.h"
#include "textfile_indexer.h"
#include "query_parser.h"

#include "config.h"

IRDatabase *database;
GtkCList *results;

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

class ResultItemGTK {
    public:
	ResultItemGTK(docid did, int percent, docname dname) {
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
	
	gchar **data;
};

static void
result_destroy_notify(gpointer data)
{
    ResultItemGTK * item = (ResultItemGTK *)data;
    delete item;
}

static void
on_query_changed(GtkWidget *widget, gpointer user_data) {
    GtkEditable *textbox = GTK_EDITABLE(widget);
    char *tmp = gtk_editable_get_chars( textbox, 0, -1);
    string query(tmp);
    g_free(tmp);

    try {
	Match matcher(database); 

	// split into terms
	QueryParser parser;
	TextfileIndexer idx;
	parser.set_indexer(&idx);
	vector<QueryTerm> qterms;
	qterms = parser.parse_query(query);

	vector<QueryTerm>::const_iterator i = qterms.begin();
	while(i != qterms.end()) {
	    matcher.add_term((*i).tname);
	    i++;
	}

	matcher.set_max_msize(10);

	matcher.match();

	weight maxweight = matcher.get_max_weight();
	doccount mtotal = matcher.mtotal;
	doccount msize = matcher.msize;

	gtk_clist_freeze(results);
	gtk_clist_clear(results);
	cout << "MTotal: " << mtotal << " Maxweight: " << maxweight << endl;
	for (docid i = 0; i < msize; i++) {
	    docid q0 = matcher.mset[i].id;
	    IRDocument *doc = database->open_document(q0);
	    IRData data = doc->get_data();
	    ResultItemGTK * item = new ResultItemGTK(matcher.mset[i].id,
		100 * matcher.mset[i].w / maxweight, data.value);
	    gint index = gtk_clist_append(results, item->data);

	    // Make sure it gets freed when item is removed from result list
	    gtk_clist_set_row_data_full(results, index, item,
					result_destroy_notify);
	}
	gtk_clist_thaw(results);

    } catch (OmError e) {
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
    try {
	database = new TextfileDatabase();
	database->open("/mnt/ivory/disk1/home/richard/textfile", true);
    } catch (OmError e) {
	cout << e.get_msg() << endl;
    }

    GladeXML *xml;
    string gladefile = "querygui.glade";

    gtk_init(&argc, &argv);
    glade_init();

    /* load the interface */
    xml = glade_xml_new(gladefile.c_str(), NULL);
    if(xml == NULL) {
	cerr << "Unable to open " << gladefile << endl;
	return 1;
    }

    /* connect the signals in the interface */
    glade_xml_signal_autoconnect(xml);

    GtkWidget * widget = glade_xml_get_widget(xml, "results");
    results = GTK_CLIST(widget);

    /* start the event loop */
    gtk_main();
    return 0;
}
