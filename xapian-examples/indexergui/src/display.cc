#include "display.h"
#include "graph.h"
#include <string>

static Graph *graph;

void
init_indexergui(GtkWidget *indexergui)
{
    graph = new Graph(indexergui);
}

void
handle_newobj()
{
//    graph->make_obj("omsplitter", "foo", 2, 3, 100.0, 100.0);
}

void
handle_open()
{
    graph->handle_open();
}

void
handle_filechosen(gpointer user_data)
{
    GtkFileSelection *filesel = GTK_FILE_SELECTION(user_data);
    gchar *fname = gtk_file_selection_get_filename(filesel);

    graph->do_open(fname);
}
