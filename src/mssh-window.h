#ifndef __MSSH_WINDOW_H__
#define __MSSH_WINDOW_H__

#include <gtk/gtk.h>
#include <vte/vte.h>

#include "mssh-terminal.h"

G_BEGIN_DECLS

#define MSSH_TYPE_WINDOW			mssh_window_get_type()
#define MSSH_WINDOW(obj)			G_TYPE_CHECK_INSTANCE_CAST(obj,\
	MSSH_TYPE_WINDOW, MSSHWindow)
#define MSSH_WINDOW_CLASS(klass)	G_TYPE_CHECK_CLASS_CAST(klass,\
	MSSH_WINDOW_TYPE, MSSHWindowClass)
#define IS_MSSH_WINDOW(obj)			G_TYPE_CHECK_INSTANCE_TYPE(obj,\
	MSSH_TYPE_WINDOW)
#define IS_MSSH_WINDOW_CLASS(klass) G_TYPE_CHECK_CLASS_TYPE(klass,\
	MSSH_TYPE_WINDOW)

typedef struct
{
	GtkWindow widget;
	GtkWidget *table;
	GtkWidget *server_menu;
	GArray *terminals;
	char **env;
	int columns;
	int timeout;
	gboolean close_ended_sessions;
	gboolean exit_on_all_closed;
} MSSHWindow;

typedef struct
{
	GtkWindowClass parent_class;
} MSSHWindowClass;

GType mssh_window_get_type(void) G_GNUC_CONST;

GtkWidget* mssh_window_new(void);
void mssh_window_start_session(MSSHWindow* window, char **env, int nhosts,
	char **servers);
void mssh_window_relayout(MSSHWindow *window);
void mssh_window_session_closed(MSSHTerminal *terminal, gpointer data);

G_END_DECLS

#endif /* __MSSH_WINDOW_H__ */
