/* querygui.cc */

#include <gtk/gtk.h>
#include <glade/glade.h>
#include <cstdio>
#include <string>
#include "querygui.h"

void on_query_changed(GtkWidget *widget, gpointer user_data) {
    
}

int main(int argc, char *argv[]) {
    GladeXML *xml;
    string gladefile = "querygui.glade";

    gtk_init(&argc, &argv);
    glade_init();

    /* load the interface */
    xml = glade_xml_new(gladefile.c_str(), NULL);
    if(xml == NULL) {
	cerr << "Unable to open " << gladefile << endl;
	return 0;
    }

    /* connect the signals in the interface */
    glade_xml_signal_autoconnect(xml);

    /* start the event loop */
    gtk_main();
    return 0;
}
