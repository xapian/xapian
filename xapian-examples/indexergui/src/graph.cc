/* graph.cc: Object which keeps track of all the state in a graph
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

#include "graph.h"
#include "om/omindexerbuilder.h"
#include "om/omindexerdesc.h"
#include "om/omerror.h"
#include <map>
extern "C" {
#include "interface.h"
}

static gint
canvas_handler(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
    Graph *graph = (Graph*)data;
    /* fprintf(stderr, "graph = %p\n", pad->parent->graph); */
    return graph->canvas_event_handler(item, event);
}

Graph::Graph(GtkWidget *indexergui_)
	: indexergui(indexergui_),
	  canvas(GNOME_CANVAS(gtk_object_get_data(GTK_OBJECT(indexergui),
						  "canvas1"))),
	  dragging_pad(false),
	  current_pad(0),
	  orig_pad(0)
{
    gtk_signal_connect(GTK_OBJECT(gnome_canvas_root(canvas)), "event",
		       GTK_SIGNAL_FUNC(canvas_handler), this);
}

Graph::~Graph()
{
    clear_graph();
}

gint
Graph::canvas_event_handler(GnomeCanvasItem *item,
			    GdkEvent *event)
{
    switch (event->type) {
	case GDK_MOTION_NOTIFY:
	    {
		if (dragging_pad) {
		    update_floating_arrow(event->motion.x, event->motion.y);
		    return TRUE;
		}
	    }
	    break;
    }
    return FALSE;
}

NodeInstance &
Graph::new_node(const std::string &id,
		const OmIndexerBuilder::NodeType &nt,
		GnomeCanvasGroup *group)
{
    nodes[id] = new NodeInstance(this, nt, group, id);
    return *nodes[id];
}

NodeInstance *
Graph::get_node(const std::string &id)
{
    if (nodes.find(id) != nodes.end()) {
	return nodes.find(id)->second;
    } else {
	return 0;
    }
}

PadInstance *
Graph::find_pad(GnomeCanvasItem *item)
{
    return pads[item];
}

static gint
do_drag(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
    static bool clicked = false;
    static double last_x, last_y;

    NodeInstance *node = reinterpret_cast<NodeInstance *>(data);
    switch (event->type) {
	case GDK_BUTTON_PRESS:
	    {
		gnome_canvas_item_raise_to_top(item);
		clicked = true;
		last_x = event->button.x;
		last_y = event->button.y;
	    }
	    return TRUE;
	case GDK_BUTTON_RELEASE:
	    clicked = false;
	    return TRUE;
	case GDK_MOTION_NOTIFY:
	    if (clicked) {
		node->move_by(event->button.x - last_x,
			      event->button.y - last_y);
		last_x = event->button.x;
		last_y = event->button.y;
	    }
	    return TRUE;
    }
    return FALSE;
}

static gint
pad_handler(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
    PadInstance *pad = reinterpret_cast<PadInstance *>(data);
    /* fprintf(stderr, "pad = %p\n", pad); */
    /* fprintf(stderr, "node = %p\n", pad->parent); */
    Graph *graph = pad->parent->graph;
    /* fprintf(stderr, "graph = %p\n", pad->parent->graph); */
    return graph->pad_event_handler(item, event, pad);
}

static gint
arrow_handler(GnomeCanvasItem *item, GdkEvent *event, gpointer data)
{
    Graph *graph = (Graph*)data;
    /* fprintf(stderr, "graph = %p\n", pad->parent->graph); */
    return graph->arrow_event_handler(item, event);
}

gint
Graph::pad_event_handler(GnomeCanvasItem *item,
			 GdkEvent *event,
			 PadInstance *pad)
{
    fprintf(stderr, "In Graph::pad_event_handler(%d)\n", event->type);
    switch (event->type) {
	case GDK_BUTTON_PRESS:
	    if (event->button.button == 1) {
		return TRUE;
	    }
	    break;
	case GDK_ENTER_NOTIFY:
	    {
		if (dragging_pad) {
		    //fprintf(stderr, "GDK_ENTER_NOTIFY: pad %p\n", pad);
		    if (orig_pad->output != pad->output) {
			current_pad = pad;
		    }
		    return TRUE;
		}
	    }
	    break;
	    /*
	case GDK_MOTION_NOTIFY:
	    {
		if (dragging_pad) {
		    if (orig_pad->output != pad->output) {
			update_floating_arrow(event->motion.x, event->motion.y);
		    }
		    return TRUE;
		}
	    }
	    break;
	    */
	case GDK_LEAVE_NOTIFY:
	    {
		if (dragging_pad) {
		    //fprintf(stderr, "GDK_LEAVE_NOTIFY: pad %p\n", pad);
		    if (current_pad == pad) {
			current_pad = 0;
		    }
		    return TRUE;
		}
	    }
	    break;
	case GDK_BUTTON_RELEASE:
	    fprintf(stderr, "button release\n");
	    if (event->button.button == 1) {
		if (dragging_pad) {
		    fprintf(stderr, "Ending drag.\n");
		    //gdk_pointer_ungrab(GDK_CURRENT_TIME);
		    dragging_pad = false;
		    handle_new_connection(orig_pad, pad);
		    return TRUE;
		} else {
		    fprintf(stderr, "Starting drag.\n");
		    dragging_pad = true;
		    orig_pad = pad;
		    current_pad = 0;
		    start_floating_arrow(orig_pad,
					 event->button.x,
					 event->button.y);

		    return TRUE;
		}
	    }
	    break;
    }
    return FALSE;
}

gint
Graph::arrow_event_handler(GnomeCanvasItem *item,
			 GdkEvent *event)
{
    switch (event->type) {
	case GDK_BUTTON_PRESS:
	    if (event->button.button == 1) {
		return TRUE;
	    }
	    break;
	case GDK_MOTION_NOTIFY:
	    {
		if (dragging_pad) {
		    update_floating_arrow(event->motion.x, event->motion.y);
		    return TRUE;
		}
	    }
	    break;
	case GDK_BUTTON_RELEASE:
	    if (event->button.button == 1) {
		gnome_canvas_item_lower_to_bottom(temp_line);
		fprintf(stderr, "lowered line...\n");

		PadInstance *this_pad = find_pad(gnome_canvas_get_item_at(
						canvas, event->button.x,
						event->button.y));
		fprintf(stderr, "found pad %p...\n", this_pad);

		gnome_canvas_item_raise_to_top(temp_line);
		fprintf(stderr, "raised line ...\n");

		if (this_pad) {
		    fprintf(stderr, "Ending drag.\n");
		    dragging_pad = false;
		    handle_new_connection(orig_pad, this_pad);
		    gnome_canvas_item_ungrab(temp_line, GDK_CURRENT_TIME);
		    gtk_signal_disconnect_by_func(GTK_OBJECT(temp_line),
						  GTK_SIGNAL_FUNC(arrow_handler),
						  this);
		}

		return TRUE;
	    }
	    break;
    }
    return FALSE;
}

void
Graph::update_floating_arrow(double x, double y)
{
    if (orig_pad->output) {
	temp_points->coords[2] = x;
	temp_points->coords[3] = y;
    } else {
	temp_points->coords[0] = x;
	temp_points->coords[1] = y;
    }
    gtk_object_set(GTK_OBJECT(temp_line),
		   "points", temp_points,
		   NULL);
}

GnomeCanvasItem *
Graph::make_arrow(double x1, double y1,
		  double x2, double y2)
{
    GnomeCanvasItem *result;
    GnomeCanvasPoints *points = gnome_canvas_points_new(2);
    points->coords[0] = x1;
    points->coords[1] = y1;
    points->coords[2] = x2;
    points->coords[3] = y2;
    result = gnome_canvas_item_new(
				   gnome_canvas_root(GNOME_CANVAS(canvas)),
				   gnome_canvas_line_get_type(),
				   "points", points,
				   "fill_color", "black",
				   "width_pixels", 2,
				   "last_arrowhead", TRUE,
				   "arrow_shape_a", 5.0,
				   "arrow_shape_b", 20.0,
				   "arrow_shape_c", 10.0,
				   NULL);
    gnome_canvas_points_unref(points);
    return result;
}

void
Graph::start_floating_arrow(PadInstance *orig,
			    double x, double y)
{
    /* FIXME: register this somewhere */
    temp_points = gnome_canvas_points_new(2);
    if (orig->output) {
	temp_points->coords[0] = x;
	temp_points->coords[1] = y;
	temp_points->coords[2] = x+10;
	temp_points->coords[3] = y+10;
    } else {
	temp_points->coords[0] = x+10;
	temp_points->coords[1] = y+10;
	temp_points->coords[2] = x;
	temp_points->coords[3] = y;
    }
    temp_line = gnome_canvas_item_new(
				      gnome_canvas_root(GNOME_CANVAS(canvas)),
				      gnome_canvas_line_get_type(),
				      "points", temp_points,
				      "fill_color", "black",
				      "width_pixels", 2,
				      "last_arrowhead", TRUE,
				      "arrow_shape_a", 5.0,
				      "arrow_shape_b", 20.0,
				      "arrow_shape_c", 10.0,
				      NULL);
    gtk_signal_connect(GTK_OBJECT(temp_line), "event",
		       GTK_SIGNAL_FUNC(arrow_handler), this);
    gnome_canvas_item_grab(temp_line,
		      GDK_POINTER_MOTION_MASK |
		      GDK_BUTTON_PRESS_MASK |
		      GDK_BUTTON_RELEASE_MASK,
		      NULL, GDK_CURRENT_TIME);
}

void
Graph::handle_new_connection(PadInstance *src,
			     PadInstance *dest)
{
    if (src == NULL || dest == NULL) {
	return;
    }
    if (src->output && dest->output) {
	gnome_error_dialog("Can't connect two output pads.");
    } else if (!src->output && !dest->output) {
	gnome_error_dialog("Can't connect two input pads.");
    } else {
	src->other_end = dest;
	dest->other_end = src;
    }
}

void
Graph::make_obj(const std::string &type, const std::string &name,
		const OmIndexerBuilder::NodeType &nt,
		double x, double y)
{
    const double width = 100.0;
    const double height = 100.0;
    GnomeCanvasGroup *root = gnome_canvas_root(GNOME_CANVAS(canvas));
    GnomeCanvasGroup *newgroup = GNOME_CANVAS_GROUP(gnome_canvas_item_new(root,
						     gnome_canvas_group_get_type(),
						     "x", x,
						     "y", y,
						     NULL));
    NodeInstance &node = new_node(name, nt, newgroup);
    node.set_pos(x, y);

    GnomeCanvasItem *item;
    item = gnome_canvas_item_new(node.get_group(),
				 gnome_canvas_rect_get_type(),
				 "x1", 0.0,
				 "y1", 0.0,
				 "x2", width,
				 "y2", height,
				 "fill_color_rgba", 0xffff0080,
				 "width_pixels", 1,
				 NULL);
    item = gnome_canvas_item_new(node.get_group(),
				 gnome_canvas_text_get_type(),
				 "x", width / 2,
				 "y", 10.0,
				 "font", "-adobe-helvetica-bold-r-normal--24-240-75-75-p-138-iso8859-1",
				 "anchor", GTK_ANCHOR_NORTH,
				 "text", name.c_str(),
				 "fill_color_rgba", 0xff,
				 "clip", 0,
				 NULL);

    int inputs = nt.inputs.size();
    int outputs = nt.outputs.size();

    for (int i=0; i<inputs; ++i) {
	make_pad(node, (width * i / inputs) + 0.5 * (width / inputs),
		        0,
			width * 0.5 / inputs,
			height / 10.0,
			false, nt.inputs[i].name);
    }

    for (int i=0; i<outputs; ++i) {
	make_pad(node, (width * i / outputs) + 0.5 * (width / outputs),
		 height,
		 width * 0.5 / outputs,
		 height / 10.0,
		 true, nt.outputs[i].name);
    }

    gtk_signal_connect(GTK_OBJECT(node.get_group()), "event",
		       GTK_SIGNAL_FUNC(do_drag), (gpointer)&node);
}

void
Graph::make_connection(const std::string &feedee,
		       const std::string &feedee_port,
		       const std::string &feeder,
		       const std::string &feeder_port)
{
    fprintf(stderr, "Making connection from %s[%s] to %s[%s]\n",
	    feeder.c_str(), feeder_port.c_str(),
	    feedee.c_str(), feedee_port.c_str());
    NodeInstance *src = get_node(feeder);
    NodeInstance *dest = get_node(feedee);

    if (!src || !dest) {
	std::string message = "Graph::make_connection(): couldn't find nodes ";
	message += feedee + " and " + feeder;
	gnome_error_dialog(message.c_str());
	return;
    }

    PadInstance *out_pad = src->get_output(feeder_port);
    PadInstance *in_pad = dest->get_input(feedee_port);
    if (!out_pad || !in_pad) {
	std::string message = "Graph::make_connection(): couldn't find pads of ";
	message += feedee + " and " + feeder;
	gnome_error_dialog(message.c_str());
	return;
    }

    GnomeCanvasItem *arrow = make_arrow(src->get_x() + out_pad->x,
					src->get_y() + out_pad->y,
					dest->get_x() + in_pad->x,
					dest->get_y() + in_pad->y);

    in_pad->arrow = arrow;
    out_pad->arrow = arrow;
}

void
Graph::make_pad(NodeInstance &node,
		double x, double y, double width, double height,
		bool output, const std::string &name)
{
    PadInstance *pad = output? node.get_output(name): node.get_input(name);
    pad->x = x;
    pad->y = y;
    GnomeCanvasPoints *points = gnome_canvas_points_new(4);
    points->coords[0] = x - width/2;
    points->coords[1] = y - height/2;
    points->coords[2] = x - width/2;
    points->coords[3] = y + height/2;
    points->coords[4] = x + width/2;
    points->coords[5] = y + height/2;
    points->coords[6] = x + width/2;
    points->coords[7] = y - height/2;
    GnomeCanvasItem *pad_widget = gnome_canvas_item_new(node.get_group(),
			  gnome_canvas_polygon_get_type(),
			  "points", points,
			  "fill_color", "red",
			  "outline_color", "black",
			  "width_pixels", 1,
			  NULL);
    gnome_canvas_points_unref(points);

    pads[pad_widget] = pad;

    gtk_signal_connect(GTK_OBJECT(pad_widget), "event",
		       GTK_SIGNAL_FUNC(pad_handler), (gpointer)pad);
}

void
Graph::handle_open()
{
    GtkWidget *filesel = create_fileselection1();

    gtk_file_selection_complete(GTK_FILE_SELECTION(filesel), "*.xml");
    gtk_widget_show(filesel);
}

void
Graph::do_open(const std::string &filename)
{
    try {
	fprintf(stderr, "Opening file %s\n", filename.c_str());

	OmIndexerDesc desc = builder.desc_from_file(filename);
	fprintf(stderr, "Opened file.\n");

	desc = builder.sort_desc(desc);

	clear_graph();
	init_graph(desc);
    } catch (OmError &e) {
	gnome_error_dialog(e.get_msg().c_str());
    }
}

void
Graph::clear_graph()
{
    std::map<std::string, NodeInstance *>::iterator i;
    for (i=nodes.begin(); i!=nodes.end(); ++i) {
	delete i->second;
    }
    nodes.clear();
}

void
Graph::init_graph(const OmIndexerDesc &desc)
{
    map<string, int> node_rows;
    map<string, int> node_cols;
    int max_row = 0;
    int max_col = 0;
    map<int, int> row_length;
    const double x_inc = 120.0;
    const double y_inc = 120.0;

    std::vector<OmIndexerDesc::NodeInstance>::const_iterator curr_node;

    /** First step, assign rows to each node.  We place each node one row
     *  below its lowest feeder.
     */
    for (curr_node = desc.nodes.begin();
	 curr_node != desc.nodes.end();
	 ++curr_node) {
	int this_node_row = 0;
	/* If any of this node's feeder nodes are in the current row,
	 * then we must increment the row.
	 */
	for (size_t i = 0; i<curr_node->input.size(); ++i) {
	    map<string, int>::const_iterator in;
	    in = node_rows.find(curr_node->input[i].feeder_node);
	    if (in != node_rows.end()) {
		if (in->second >= this_node_row) {
		    this_node_row = in->second + 1;
		}
	    }
	}
	node_rows[curr_node->id] = this_node_row;
	node_cols[curr_node->id] = row_length[this_node_row]++;
	if (this_node_row > max_row) {
	    max_row = this_node_row;
	}
	if (row_length[this_node_row] > max_col) {
	    max_col = row_length[this_node_row];
	}
    }

    for (curr_node = desc.nodes.begin();
	 curr_node != desc.nodes.end();
	 ++curr_node) {
	fprintf(stderr, "Making node %s at %d, %d\n", curr_node->id.c_str(),
		node_cols[curr_node->id], node_rows[curr_node->id]);

	OmIndexerBuilder::NodeType nt = builder.get_node_info(curr_node->type);

	make_obj(curr_node->type, curr_node->id,
		 nt,
		 node_cols[curr_node->id] * x_inc,
		 node_rows[curr_node->id] * y_inc);

	for (size_t i=0; i<curr_node->input.size(); ++i) {
	    make_connection(curr_node->id,
			    curr_node->input[i].input_name,
			    curr_node->input[i].feeder_node,
			    curr_node->input[i].feeder_out);
	}
    }
    gnome_canvas_set_scroll_region(canvas, -50, -50,
				   (max_col+1) * x_inc + 50,
				   (max_row+1) * y_inc + 50);
}
