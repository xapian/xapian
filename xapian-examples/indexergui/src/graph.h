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
#include "om/omindexerdesc.h"
#include "om/omindexerbuilder.h"

class Graph {
    public:
	/** Create an empty graph */
	Graph(GtkWidget *indexergui);

	/** Delete data */
	~Graph();

	/** Add a new object (node) to the graph */
	void make_obj(const std::string &type, const std::string &name,
		      const OmIndexerBuilder::NodeType &nt,
		      double x, double y);

	/** Add a new connection between two nodes */
	void make_connection(const std::string &feedee,
			     const std::string &feedee_port,
			     const std::string &feeder,
			     const std::string &feeder_port);

	/** Add a pad to an object */
	void make_pad(NodeInstance &node,
		      double x, double y,
		      double width, double height,
		      bool output, const std::string &name);

	/** Open an indexer description file. */
	void do_open(const std::string &filename);

	/** Create a file open dialog (used when the Open button is pressed)*/
	void handle_open();

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

	NodeInstance &new_node(const std::string &id,
			       const OmIndexerBuilder::NodeType &nt,
			       GnomeCanvasGroup *group);
	NodeInstance *get_node(const std::string &id);

	std::map<std::string, NodeInstance *> nodes;

	std::map<GnomeCanvasItem *, PadInstance *> pads;

	PadInstance *find_pad(GnomeCanvasItem *item);

	/** Clear all graph elements */
	void clear_graph();

	/** Initialise the display with the graph description */
	void init_graph(const OmIndexerDesc &desc);

	GnomeCanvasItem *make_arrow(double x1, double y1,
				    double x2, double y2);
	/* Update the floating end of the moving arrow */
	void update_floating_arrow(double x, double y);

	/* Update the floating end of the moving arrow */
	void start_floating_arrow(PadInstance *orig,
				     double x, double y);

	/** Used for interfacing with the indexer building system. */
	OmIndexerBuilder builder;

	/* state used while dragging between input/output pads */
	bool dragging_pad;
	PadInstance *current_pad;
	PadInstance *orig_pad;
	GnomeCanvasItem *temp_line;
	GnomeCanvasPoints *temp_points;
	guint32 grab_time;
};

#endif /* HGUARD_OMINDEXERGUI_GRAPH_H */
