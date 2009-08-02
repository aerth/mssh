#ifndef __MSSH_WINDOW_H__
#define __MSSH_WINDOW_H__

#include <gtk/gtk.h>
#include <vte/vte.h>

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
	GtkWidget *vbox;
	GtkWidget *entry;
	GtkWidget *table;
	GtkWidget *menu_bar;
	GtkWidget *server_menu;
	GtkWidget *file_menu;
	GtkWidget *server_item;
	GtkWidget *file_item;
	GtkWidget *file_quit;
	char **env;
	char **servers;
	int num_servers;
	GtkWidget *items[32];
	GtkWidget *terms[32];
} MSSHWindow;

typedef struct
{
	GtkWindowClass parent_class;
} MSSHWindowClass;

GType mssh_window_get_type(void) G_GNUC_CONST;

GtkWidget* mssh_window_new(void);
void mssh_window_new_session(MSSHWindow* window, char **env,
	int num_servers, char **servers);

G_END_DECLS

#endif /* __MSSH_WINDOW_H__ */
