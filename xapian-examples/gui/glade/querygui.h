/* querygui.h: Header for callbacks in test GUI for om
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

#ifndef _querygui_h_
#define _querygui_h_

#ifdef __cplusplus
extern "C" {
#endif

#include <gtk/gtk.h>
#include <glade/glade.h>

void on_query_changed(GtkWidget *widget, gpointer user_data);
void on_results_selection(GtkWidget *widget,
			  gint row,
			  gint column,
			  GdkEventButton *event,
			  gpointer data);


gboolean on_mainwindow_destroy(GtkWidget *widget,
			       GdkEvent *event,
			       gpointer user_data);

#ifdef __cplusplus
}
#endif

#endif /* _querygui_h_ */
