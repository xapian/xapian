/* querygui.h */

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
