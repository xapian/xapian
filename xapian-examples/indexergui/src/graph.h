/* graph.h: Object which keeps track of all the state in a graph
 * as it is being edited.
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

#ifndef HGUARD_OMINDEXERGUI_GRAPH_H
#define HGUARD_OMINDEXERGUI_GRAPH_H

#include <gtk/gtk.h>
#include <gnome.h>
#include <string>
#include <vector>
#include <map>
#include "nodeinstance.h"

class Graph {
    public:
	/** Create an empty graph */
	Graph(GtkWidget *indexergui);

	/** Delete data */
	~Graph();

	/** Add a new object (node) to the graph */
	void make_obj(const std::string &type, const std::string &name,
		      int inputs, int outputs);

	/** Add a pad to an object */
	void make_pad(NodeInstance &node,
		      double x, double y,
		      double width, double height,
		      bool output, int index);

	/** Handle mouse events on a node pad */
	gint pad_event_handler(GnomeCanvasItem *item,
			       GdkEvent *event,
			       PadInstance *pad);

	/** Handle mouse events on the floating arrow */
	gint arrow_event_handler(GnomeCanvasItem *item,
			       GdkEvent *event);

	/** Handle mouse events on the canvas as a whole */
	gint canvas_event_handler(GnomeCanvasItem *item,
				  GdkEvent *event);
	/** Handle a new pad-to-pad connection (with error checking) */
	void handle_new_connection(PadInstance *src, PadInstance *dest);
    private:
	GtkWidget *indexergui;
	GnomeCanvas *canvas;

	NodeInstance &new_node(int numinputs, int numoutputs,
			       GnomeCanvasGroup *group);
	NodeInstance &get_node(int index);

	std::vector<NodeInstance *> nodes;

	std::map<GnomeCanvasItem *, PadInstance *> pads;

	PadInstance *find_pad(GnomeCanvasItem *item);

	/* Update the floating end of the moving arrow */
	void update_floating_arrow(double x, double y);

	/* Update the floating end of the moving arrow */
	void start_floating_arrow(PadInstance *orig,
				     double x, double y);

	/* state used while dragging between input/output pads */
	bool dragging_pad;
	PadInstance *current_pad;
	PadInstance *orig_pad;
	GnomeCanvasItem *temp_line;
	GnomeCanvasPoints *temp_points;
	guint32 grab_time;
};

#endif /* HGUARD_OMINDEXERGUI_GRAPH_H */
