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

#include "config.h"

IRDatabase *database;

void on_query_changed(GtkWidget *widget, gpointer user_data) {
    GtkEditable *textbox = GTK_EDITABLE(widget);
    char *tmp = gtk_editable_get_chars( textbox, 0, -1);
    string query(tmp);
    g_free(tmp);

    try {
	Match matcher(database); 

	// FIXME - split into terms
	matcher.add_term(query);

	matcher.set_max_msize(10);

	matcher.match();

	weight maxweight = matcher.get_max_weight();
	doccount mtotal = matcher.mtotal;
	doccount msize = matcher.msize;

	cout << "MTotal: " << mtotal << " Maxweight: " << maxweight << endl;
	for (docid i = 0; i < msize; i++) {
	    docid q0 = matcher.mset[i].id;
	    IRDocument *doc = database->open_document(q0);
	    IRData data = doc->get_data();
	    string p = data.value;
	    cout << q0 << ":[" << p << "] " << matcher.mset[i].w << "\n\n";
	}

    } catch (OmError e) {
	cout << e.get_msg() << endl;
    }
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

    /* start the event loop */
    gtk_main();
    return 0;
}
