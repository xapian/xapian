/* nodeinstance.h: Description of object corresponding to a node in the display.
 *
 * ----START-LICENCE----
 * Copyright 1999,2000,2001 BrightStation PLC
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

#ifndef HGUARD_OMINDEXERGUI_NODE_H
#define HGUARD_OMINDEXERGUI_NODE_H

#include <vector>
#include <string>
#include <om/omindexerbuilder.h>

class NodeInstance;
class Graph;

struct PadInstance {
    NodeInstance *parent;
    std::string name;
    bool output;
    PadInstance *other_end;
    GnomeCanvasItem *arrow;

    /* Co-ordinates relative to the containing node */
    double x;
    double y;

    PadInstance(NodeInstance *parent_, bool output_)
	    : parent(parent_), output(output_), other_end(0), arrow(0) {
		fprintf(stderr, "Making new %s pad = %p\n",
			output? "output" : "input", this);
	    }

    PadInstance(const PadInstance &other)
	    : parent(other.parent),
    	      name(other.name),
	      output(other.output),
	      other_end(other.other_end), arrow(other.arrow) {}
};

class NodeInstance {
    public:
    	NodeInstance(Graph *graph_, const OmIndexerBuilder::NodeType &nt,
		     GnomeCanvasGroup *group_, const std::string &name_)
	    : graph(graph_), group(group_), id(name_)
	{
	    fprintf(stderr, "New NodeInstance(%p, %d, %d) = %p\n",
		    graph, nt.inputs.size(), nt.outputs.size(), this);
	    for (int i=0; i<nt.inputs.size(); ++i) {
		new_input(nt.inputs[i].name);
	    }
	    for (int i=0; i<nt.outputs.size(); ++i) {
		new_output(nt.outputs[i].name);
	    }
	}

	~NodeInstance() {
	    std::map<std::string, PadInstance *>::iterator i;
	    for (i=inputs.begin(); i != inputs.end(); ++i) {
		delete i->second;
	    }
	    for (i=outputs.begin(); i != outputs.end(); ++i) {
		delete i->second;
	    }
	    gtk_object_destroy(GTK_OBJECT(group));
	}

	void move_by(double right, double down) {
	    gnome_canvas_item_move(GNOME_CANVAS_ITEM(group), right, down);

	    /* Move the connected arrows too */
	    std::map<std::string, PadInstance *>::iterator i;
	    for (i=inputs.begin(); i!=inputs.end(); ++i) {
		if (i->second->arrow) {
		    GnomeCanvasPoints *points;

		    gtk_object_get(GTK_OBJECT(i->second->arrow),
				   "points", &points,
				   NULL);
		    points->coords[2] += right;
		    points->coords[3] += down;
		    gtk_object_set(GTK_OBJECT(i->second->arrow),
				   "points", points,
				   NULL);
		}
	    }
	    for (i=outputs.begin(); i!=outputs.end(); ++i) {
		if (i->second->arrow) {
		    GnomeCanvasPoints *points;

		    gtk_object_get(GTK_OBJECT(i->second->arrow),
				   "points", &points,
				   NULL);
		    points->coords[0] += right;
		    points->coords[1] += down;
		    gtk_object_set(GTK_OBJECT(i->second->arrow),
				   "points", points,
				   NULL);
		}
	    }
	}

	GnomeCanvasGroup *get_group() const {
	    return group;
	}
	PadInstance *get_input(const std::string &name) {
	    std::map<std::string, PadInstance *>::const_iterator i;
	    i = inputs.find(name);
	    if (i == inputs.end()) {
		fprintf(stderr, "Trying to get input pad %s[%s]: failed\n",
			id.c_str(), name.c_str());
		return 0;
	    } else {
		fprintf(stderr, "Trying to get input pad %s[%s]: %p\n",
			id.c_str(), name.c_str(), i->second);
		return i->second;
	    }
	}
	PadInstance *get_output(const std::string &name) {
	    std::map<std::string, PadInstance *>::const_iterator i;
	    i = outputs.find(name);
	    if (i == outputs.end()) {
		fprintf(stderr, "Trying to get output pad %s[%s]: failed\n",
			id.c_str(), name.c_str());
		return 0;
	    } else {
		fprintf(stderr, "Trying to get output pad %s[%s]: %p\n",
			id.c_str(), name.c_str(), i->second);
		return i->second;
	    }
	}

	void set_pos(double x_, double y_) {
	    x = x_;
	    y = y_;
	}

	double get_x() const {
	    return x;
	}
	double get_y() const {
	    return y;
	}

	Graph *graph;
    private:
	PadInstance &new_input(const std::string &name) {
	    inputs[name] = new PadInstance(this, false);
	    fprintf(stderr, "Added input number %d\n", inputs.size());
	    return *inputs[name];
	};
	PadInstance &new_output(const std::string &name) {
	    outputs[name] = new PadInstance(this, true);
	    fprintf(stderr, "Added output number %d\n", outputs.size());
	    return *outputs[name];
	};

    	GnomeCanvasGroup *group;

	std::string id;

	double x;
	double y;

	std::map<std::string, PadInstance *> inputs;
	std::map<std::string, PadInstance *> outputs;
};
#endif /* HGUARD_OMINDEXERGUI_NODE_H */
