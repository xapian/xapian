#include <gnome.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void init_indexergui(GtkWidget *);
void handle_newobj();
void handle_open();
void handle_filechosen(gpointer user_data);

#ifdef __cplusplus
}
#endif /* __cplusplus */
