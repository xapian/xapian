# Copyright (C) 2008 Lemur Consulting Ltd
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program; if not, write to the Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
"""templates.py: Template reading and rendering code.

"""

import HTMLTemplate
import os

template_dir = os.path.join(os.path.dirname(__file__), "templates")

__all__ = (
           'render_template',
)

def render_template(render_fn, name, *args):
    """Get a template.

    `render_fn` is the render function to use.
    `name` is the file name for the template.
    `args` is any arguments to supply to the render function.

    """
    template_path = os.path.join(template_dir, name)
    t = HTMLTemplate.Template(render_fn, open(template_path, "rb").read())
    return t.render(*args)
