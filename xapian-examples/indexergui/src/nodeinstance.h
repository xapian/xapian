/* nodeinstance.h: Description of object corresponding to a node in the display.
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

#ifndef HGUARD_OMINDEXERGUI_NODE_H
#define HGUARD_OMINDEXERGUI_NODE_H

#include <vector>
#include <string>

class NodeInstance;
class Graph;

struct PadInstance {
    NodeInstance *parent;
    std::string name;
    bool output;
    PadInstance *other_end;

    PadInstance(NodeInstance *parent_, bool output_)
	    : parent(parent_), output(output_), other_end(0) {
		fprintf(stderr, "Making new %s pad = %p\n",
			output? "output" : "input", this);
	    }

    PadInstance(const PadInstance &other)
	    : parent(other.parent),
    	      name(other.name),
	      output(other.output),
	      other_end(other.other_end) {}
};

class NodeInstance {
    public:
    	NodeInstance(Graph *graph_, int numinputs, int numoutputs,
		     GnomeCanvasGroup *group_)
	    : graph(graph_), group(group_)
	{
	    fprintf(stderr, "New NodeInstance(%p, %d, %d) = %p\n",
		    graph, numinputs, numoutputs, this);
	    for (int i=0; i<numinputs; ++i) {
		new_input();
	    }
	    for (int i=0; i<numoutputs; ++i) {
		new_output();
	    }
	}

	~NodeInstance() {
	    std::vector<PadInstance *>::iterator i;
	    for (i=inputs.begin(); i != inputs.end(); ++i) {
		delete *i;
	    }
	    for (i=outputs.begin(); i != outputs.end(); ++i) {
		delete *i;
	    }
	}

	Graph *graph;

	GnomeCanvasGroup *get_group() const {
	    return group;
	}
	PadInstance &get_input(int index) {
	    return *inputs[index];
	}
	PadInstance &get_output(int index) {
	    return *outputs[index];
	}
    private:
	PadInstance &new_input() {
	    inputs.push_back(new PadInstance(this, false));
	    fprintf(stderr, "Added input number %d\n", inputs.size());
	    return *inputs[inputs.size() - 1];
	};
	PadInstance &new_output() {
	    outputs.push_back(new PadInstance(this, true));
	    fprintf(stderr, "Added output number %d\n", outputs.size());
	    return *outputs[outputs.size() - 1];
	};

    	GnomeCanvasGroup *group;

	std::vector<PadInstance *> inputs;
	std::vector<PadInstance *> outputs;
};
#endif /* HGUARD_OMINDEXERGUI_NODE_H */
