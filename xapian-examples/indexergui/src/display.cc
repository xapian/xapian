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
handle_newobj() {
    graph->make_obj("omsplitter", "foo", 2, 3);
}
