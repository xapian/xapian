#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"
#include "display.h"


void
on_exit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data)
{
    gtk_main_quit();
}


void
on_button_newobj_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
    handle_newobj();
}

